#include <shutter.hpp>

// Predefined controller profiles for your convenience.
// Profiles are important because not every controller has the same amount of channels
// and their timings differ.
shutter::ControllerProfile
    PROFILE_TIMER_8K((ShutterIndex)8, chrono_ms(30), chrono_ms(2500), chrono_ms(3500)),
    PROFILE_HANDHELD_TRANSMITTER((ShutterIndex)8, chrono_ms(100), chrono_ms(2000), chrono_ms(4000));
