#include <shutter.hpp>

// Predefined controller profiles for your convenience.
// Profiles are important because not every controller has the same amount of channels
// and their timings differ.
//
// The constructor used here uses the following order:
// - Amount of shutters (channels) the controller can control
// - Duration to press the select next / previous button
// - Duration in milliseconds for which the shutter selection is guaranteed to be active.
// - Duration in milliseconds AFTER which the shutter selection is guaranteed to be inactive.
//
// Here are a few debugging tips:
//
// - If the selected shutter doesn't change the 'select_duration' (2nd argument) is too small.
//   Increase it until the selection changes.
//
// - If the controller fails to select the correct shutter the 3rd and 4th arguments need adjustment.
//   Measure the time it takes for the shutter selection to turn off after pressing a button multiple times.
//   The 3rd argument should be lower than the lowest value you recorded (doesn't hurt if it's way too low)
//   and the 4th argument should be bigger than the highest value.
//
//   For testing purposes you can set the 3rd argument to 0 and the 4th to something high like 5000ms.
//   However please note that the 3rd value shouldn't remain 0 as it would drastically slow down the shutter selection.
shutter::ControllerProfile
    PROFILE_TIMER_8K((ShutterIndex)8, chrono_ms(30), chrono_ms(2000), chrono_ms(4500)),
    PROFILE_HANDHELD_TRANSMITTER((ShutterIndex)8, chrono_ms(100), chrono_ms(2000), chrono_ms(4500));
