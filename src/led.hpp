#ifndef led_ns
#define led_ns

#include <Arduino.h>

namespace led
{
    const uint8_t PIN_LED = 2;

    void setup()
    {
        pinMode(PIN_LED, OUTPUT);
    }

    void set_state(bool on)
    {
        digitalWrite(PIN_LED, on);
    }

    void flash_blocking(uint32_t on_ms, uint32_t off_ms)
    {
        set_state(true);
        delay(on_ms);
        set_state(false);
        delay(off_ms);
    }
    void flash_blocking(uint32_t delay_ms)
    {
        auto half_ms = delay_ms / 2;
        flash_blocking(half_ms, half_ms);
    }

    void flash_ok()
    {
        flash_blocking(500, 1500);
    }
    void flash_err()
    {
        flash_blocking(1500, 500);
    }
} // namespace led
#endif