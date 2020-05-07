#include <config_profiles.hpp>

#define WIFI_SSID "Your WiFi SSID"
#define WIFI_PASSWORD "Your Wifi Password"

#define MQTT_SERVER_DOMAIN "localhost"
#define MQTT_SERVER_PORT 1883
// Client id used to communicate with the MQTT server
#define MQTT_CLIENT_ID "shutter-control"

// This is the list of controllers which are attached to the board.
// A controller is defined by a profile followed by the pins
// controlling the buttons in the following order:
//      UP, STOP, DOWN, PREVIOUS, NEXT
//
// DON'T reuse the same pin for multiple controllers or buttons.
//
// If you need to define a new profile, see the `config_profiles.hpp` file
//
// The order of the list is important because the shutters from a controller
// start with the next index after the previous controller's last shutter.
// i.e. if the first controller has 8 shutters the first shutter of the second controller has index 8.
shutter::Controller CONTROLLERS[] = {
    {PROFILE_TIMER_8K, 26, 25, 33, 22, 23},
    {PROFILE_HANDHELD_TRANSMITTER, 13, 12, 14, 0, 15},
};
