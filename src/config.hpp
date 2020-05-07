#include <config_profiles.hpp>

#define WIFI_SSID "Your WiFi SSID"
#define WIFI_PASSWORD "Your Wifi Password"

#define MQTT_SERVER_DOMAIN "localhost"
#define MQTT_SERVER_PORT 1883
// Client id used to communicate with the MQTT server
#define MQTT_CLIENT_ID "shutter-control"

// Pin order: UP, STOP, DOWN, PREVIOUS, NEXT
shutter::Controller CONTROLLERS[] = {
    {PROFILE_HANDHELD_TRANSMITTER, 13, 12, 14, 0, 15},
};
