#include <mutex>
#include <thread>

#include <Arduino.h>

#include <arith.hpp>
#include <shutter.hpp>

#define chrono_ms std::chrono::milliseconds

const chrono_ms PRESS_DURATION(30);
const chrono_ms RECOVERY_DURATION(30);

const chrono_ms SEND_DURATION(1600);
const chrono_ms SEND_PAUSE_DURATION(750);
const uint SEND_CMD_COUNT = 3;

const chrono_ms SHUTTER_DISPLAY_ON_MIN(2500);
const chrono_ms SHUTTER_DISPLAY_OFF_MAX(3500);

const chrono_ms MIN_SLEEP_FOR_UNLOCK(10000);
const chrono_ms EARLY_WAKEUP_FOR_LOCK(3500);

const uint8_t BTN_UP_PIN = 13, BTN_DOWN_PIN = 12;
const uint8_t BTN_NEXT_PIN = 0, BTN_PREV_PIN = 15;

// globals
std::mutex g_remoteControlLock;
uint8_t g_selectedShutter = 0;
uint32_t g_lastShutterSelectionActiveTime;

void setupPins()
{
  for (auto const &pin : {BTN_UP_PIN, BTN_DOWN_PIN, BTN_NEXT_PIN, BTN_PREV_PIN})
    pinMode(pin, OUTPUT);
}

void pressButton(uint8_t pin, chrono_ms duration)
{
  digitalWrite(pin, HIGH);
  std::this_thread::sleep_for(duration);
  digitalWrite(pin, LOW);

  // whenever ANY button is pressed the selection display goes active
  g_lastShutterSelectionActiveTime = millis();
}

void pressButton(uint8_t pin, chrono_ms duration, uint count, chrono_ms pause)
{
  if (count == 0)
    return;

  pressButton(pin, duration);
  for (auto i = 1; i < count; i++)
  {
    std::this_thread::sleep_for(pause);
    pressButton(pin, duration);
  }
}

void _selectNextShutter()
{
  pressButton(BTN_NEXT_PIN, PRESS_DURATION);
}

void _selectPreviousShutter()
{
  pressButton(BTN_PREV_PIN, PRESS_DURATION);
}

void maybeWakeUpShutterSelection()
{
  const chrono_ms elapsed(millis() - g_lastShutterSelectionActiveTime);
  if (elapsed < SHUTTER_DISPLAY_ON_MIN)
    // still active!
    return;

  if (elapsed < SHUTTER_DISPLAY_OFF_MAX)
    // not certain whether still active, delay until we're sure it's off.
    std::this_thread::sleep_for(SHUTTER_DISPLAY_OFF_MAX - elapsed);

  // this doesn't change the selection, it only wakes the display.
  _selectNextShutter();
}

void selectShutterUnsafe(uint8_t shutter)
{
  const uint8_t HALFWAY_POINT = TOTAL_SHUTTERS / 2;

  auto step_f = _selectNextShutter;
  auto steps = subModN(shutter, g_selectedShutter, TOTAL_SHUTTERS);
  if (steps == 0)
    return;

  if (steps > HALFWAY_POINT)
  {
    step_f = _selectPreviousShutter;
    steps = TOTAL_SHUTTERS - steps;
  }

  maybeWakeUpShutterSelection();

  for (auto i = 0; i < steps; i++)
  {
    std::this_thread::sleep_for(RECOVERY_DURATION);
    step_f();
  }

  g_selectedShutter = shutter;
}

void shutterUpUnsafe(uint8_t shutter, uint signalCount = SEND_CMD_COUNT)
{
  Serial.printf("rolling up shutter %u\n", shutter);
  selectShutterUnsafe(shutter);
  pressButton(BTN_UP_PIN, SEND_DURATION, signalCount, SEND_PAUSE_DURATION);
}

void shutterUp(uint8_t shutter)
{
  std::lock_guard<std::mutex> guard(g_remoteControlLock);
  shutterUpUnsafe(shutter);
}

void shutterDownUnsafe(uint8_t shutter, uint signalCount = SEND_CMD_COUNT)
{
  Serial.printf("rolling down shutter %u\n", shutter);
  selectShutterUnsafe(shutter);
  pressButton(BTN_DOWN_PIN, SEND_DURATION, signalCount, SEND_PAUSE_DURATION);
}

void shutterDown(uint8_t shutter)
{
  std::lock_guard<std::mutex> guard(g_remoteControlLock);
  shutterDownUnsafe(shutter);
}

void threadSleepWithSmartUnlock(chrono_ms timeout)
{
  if (timeout < MIN_SLEEP_FOR_UNLOCK)
  {
    std::this_thread::sleep_for(timeout);
    return;
  }

  auto wakeupTime = std::chrono::system_clock::now() + timeout;
  g_remoteControlLock.unlock();
  std::this_thread::sleep_for(timeout - EARLY_WAKEUP_FOR_LOCK);
  g_remoteControlLock.lock();
  std::this_thread::sleep_until(wakeupTime);
}

void shutterRollFromTop(uint8_t shutter, chrono_ms time)
{
  shutterUp(shutter);
  // TODO wait for roll up (without lock)
  std::lock_guard<std::mutex> guard(g_remoteControlLock);
  shutterDownUnsafe(shutter);
  threadSleepWithSmartUnlock(time);
  // send command once to stop
  shutterUpUnsafe(shutter, 1);
}
