#include <mutex>
#include <thread>

#include <Arduino.h>

#include <arith.hpp>

namespace shutter
{
#define ShutterIndex uint8_t
#define chrono_ms std::chrono::milliseconds
#define time_now std::chrono::system_clock::now

    class ControllerProfile
    {

    public:
        ShutterIndex shutters;

        chrono_ms select_duration;
        chrono_ms select_recovery_duration;

        chrono_ms send_duration;
        chrono_ms send_recovery_duration;
        uint send_count;

        chrono_ms selection_active_duration_min;
        chrono_ms selection_active_duration_max;

        ControllerProfile(ShutterIndex shutters, chrono_ms select,
                          chrono_ms active_min, chrono_ms active_max)
            : shutters(shutters),
              select_duration(select), select_recovery_duration(select),
              send_duration(2500), send_recovery_duration(750),
              send_count(2),
              selection_active_duration_min(active_min), selection_active_duration_max(active_max){};
    };

    class ControllerButton
    {
        uint8_t m_pin;

    public:
        ControllerButton(uint8_t pin) : m_pin(pin){};

        void setup() const
        {
            pinMode(m_pin, OUTPUT);
        }

        void press(chrono_ms duration) const
        {
            digitalWrite(m_pin, HIGH);
            std::this_thread::sleep_for(duration);
            digitalWrite(m_pin, LOW);
        }

        void press_repeat(chrono_ms duration, uint count, chrono_ms pause) const
        {
            if (count == 0)
                return;

            press(duration);
            for (auto i = 1; i < count; i++)
            {
                std::this_thread::sleep_for(pause);
                press(duration);
            }
        }
    };

    struct ShutterProfile
    {
        ShutterIndex index;
        chrono_ms total_time;
    };

    class Controller
    {
        ControllerProfile m_profile;
        ControllerButton m_up, m_stop, m_down, m_previous, m_next;

        std::mutex m_controller_lock;
        ShutterIndex m_selected_shutter;
        unsigned long m_last_selection_active_at;

        void _select_previous_shutter()
        {
            m_previous.press(m_profile.select_duration);
            m_selected_shutter = arith::sub_modn(m_selected_shutter, (ShutterIndex)1, m_profile.shutters);
            m_last_selection_active_at = millis();
        }

        void _select_next_shutter()
        {
            m_next.press(m_profile.select_duration);
            m_selected_shutter = arith::add_modn(m_selected_shutter, (ShutterIndex)1, m_profile.shutters);
            m_last_selection_active_at = millis();
        }

        void _ensure_selection_active()
        {
            const chrono_ms elapsed(millis() - m_last_selection_active_at);
            if (elapsed < m_profile.selection_active_duration_min)
                // still active!
                return;

            if (elapsed < m_profile.selection_active_duration_max)
                // not certain whether still active, delay until we're sure it's off.
                std::this_thread::sleep_for(m_profile.selection_active_duration_max - elapsed);

            // this doesn't change the selection, it only makes it active.
            m_next.press(m_profile.select_duration);
            m_last_selection_active_at = millis();
        }

        void _select_shutter(ShutterIndex shutter)
        {
            const ShutterIndex TOTAL_SHUTTERS = m_profile.shutters;
            const ShutterIndex HALFWAY_POINT = TOTAL_SHUTTERS / 2;

            auto forwards = true;
            auto steps = arith::sub_modn(shutter, m_selected_shutter, TOTAL_SHUTTERS);
            if (steps == 0)
                return;

            if (steps > HALFWAY_POINT)
            {
                forwards = false;
                steps = TOTAL_SHUTTERS - steps;
            }

            _ensure_selection_active();

            for (auto i = 0; i < steps; i++)
            {
                std::this_thread::sleep_for(m_profile.select_recovery_duration);
                if (forwards)
                    _select_next_shutter();
                else
                    _select_previous_shutter();
            }
        }

        void _press_up(ShutterIndex shutter, uint count)
        {
            _select_shutter(shutter);
            m_up.press_repeat(m_profile.send_duration, count, m_profile.send_recovery_duration);
            m_last_selection_active_at = millis();
        }

        void _press_stop(ShutterIndex shutter, uint count)
        {
            _select_shutter(shutter);
            m_stop.press_repeat(m_profile.send_duration, count, m_profile.send_recovery_duration);
            m_last_selection_active_at = millis();
        }

        void _press_down(ShutterIndex shutter, uint count)
        {
            _select_shutter(shutter);
            m_down.press_repeat(m_profile.send_duration, count, m_profile.send_recovery_duration);
            m_last_selection_active_at = millis();
        }

        template <typename T>
        void _sleep_until(T s)
        {
            const chrono_ms MIN_SLEEP_FOR_UNLOCK(15000);
            const chrono_ms EARLY_WAKEUP_FOR_LOCK(4000);

            auto timeout = s - time_now();
            if (timeout < MIN_SLEEP_FOR_UNLOCK)
            {
                std::this_thread::sleep_until(s);
                return;
            }

            Serial.println("[smart lock] unlocking");
            m_controller_lock.unlock();
            std::this_thread::sleep_for(timeout - EARLY_WAKEUP_FOR_LOCK);
            Serial.println("[smart lock] reacquiring lock");
            m_controller_lock.lock();
            std::this_thread::sleep_until(s);
        }

    public:
        Controller(ControllerProfile profile,
                   ControllerButton up, ControllerButton stop, ControllerButton down,
                   ControllerButton previous, ControllerButton next)
            : m_profile(profile),
              m_up(up), m_stop(stop), m_down(down),
              m_previous(previous), m_next(next){};

        Controller(ControllerProfile profile,
                   uint8_t up, uint8_t stop, uint8_t down,
                   uint8_t previous, uint8_t next)
            : Controller(profile,
                         ControllerButton(up), ControllerButton(stop), ControllerButton(down),
                         ControllerButton(previous), ControllerButton(next)){};

        void setup() const
        {
            for (auto &btn : {m_up, m_stop, m_down, m_previous, m_next})
            {
                btn.setup();
            }
        }

        uint total_shutters()
        {
            return m_profile.shutters;
        }

        void roll_up(ShutterIndex shutter)
        {
            std::lock_guard<std::mutex> guard(m_controller_lock);
            _press_up(shutter, m_profile.send_count);
        }

        void roll_up(ShutterIndex shutter, chrono_ms time)
        {
            std::lock_guard<std::mutex> guard(m_controller_lock);
            auto sleep_until = time_now() + time;
            _press_up(shutter, m_profile.send_count);
            _sleep_until(sleep_until);
            _press_stop(shutter, m_profile.send_count);
        }

        void roll_stop(ShutterIndex shutter)
        {
            std::lock_guard<std::mutex> guard(m_controller_lock);
            _press_stop(shutter, m_profile.send_count);
        }

        void roll_down(ShutterIndex shutter)
        {
            std::lock_guard<std::mutex> guard(m_controller_lock);
            _press_down(shutter, m_profile.send_count);
        }

        void roll_down(ShutterIndex shutter, chrono_ms time)
        {
            std::lock_guard<std::mutex> guard(m_controller_lock);
            auto sleep_until = time_now() + time;
            _press_down(shutter, m_profile.send_count);
            _sleep_until(sleep_until);
            _press_stop(shutter, m_profile.send_count);
        }

        void roll_from_top(ShutterProfile shutter, chrono_ms time)
        {
            roll_up(shutter.index);
            std::this_thread::sleep_for(shutter.total_time);
            roll_down(shutter.index, time);
        }

        void roll_from_bottom(ShutterProfile shutter, chrono_ms time)
        {
            roll_down(shutter.index);
            std::this_thread::sleep_for(shutter.total_time);
            roll_up(shutter.index, time);
        }

        void roll_to(ShutterProfile shutter, double percentage)
        {
            if (percentage < 0.0 || percentage > 1.0)
                return;

            auto from_top = true;
            if (percentage > 0.5)
            {
                from_top = false;
                percentage = 1.0 - percentage;
            }

            auto time = std::chrono::duration_cast<chrono_ms>(percentage * shutter.total_time);
            if (from_top)
                roll_from_top(shutter, time);
            else
                roll_from_bottom(shutter, time);
        }
    };
} // namespace shutter
