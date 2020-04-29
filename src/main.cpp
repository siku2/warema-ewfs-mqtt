#include <thread>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <shutter.hpp>

#if !(defined(WIFI_SSID) && defined(WIFI_PASSWORD))
#error WIFI_SSID and WIFI_PASSWORD must be defined
#endif

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "shutter-control"
#endif

#ifndef MQTT_SERVER_DOMAIN
#define MQTT_SERVER_DOMAIN "localhost"
#endif

#ifndef MQTT_SERVER_PORT
#define MQTT_SERVER_PORT 1883
#endif

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void connectWifi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttSubscribe()
{
  Serial.println("Subscribing to MQTT topics");
  mqttClient.subscribe("ewfs/command");
}

void connectMQTT()
{
  Serial.println("Connecting to MQTT");
  if (!mqttClient.connect(MQTT_CLIENT_ID))
    return;

  mqttSubscribe();
}

void publishShutterState(uint8_t shutter, String state)
{
  StaticJsonDocument<256> doc;
  doc["assumed_state"] = state;

  char buf[256];
  auto n = serializeJson(doc, buf);

  char topicBuf[32];
  sprintf(topicBuf, "ewfs/shutters/%u", shutter);
  mqttClient.publish(topicBuf, buf, n);
}

void onMQTTMessage(char *topic, byte *payload, unsigned int length)
{
  StaticJsonDocument<256> doc;
  const auto err = deserializeJson(doc, payload, length);
  if (err)
  {
    Serial.print("failed to deserialize message: ");
    Serial.println(err.c_str());
    return;
  }

  const char *op = doc["op"];
  const uint8_t shutter = doc["shutter"];
  Serial.printf("OP: %s | SHUTTER: %u\n", op, shutter);

  if (shutter >= TOTAL_SHUTTERS)
  {
    Serial.print("invalid shutter: ");
    Serial.println(shutter);
    return;
  }

  if (strcmp(op, "shutter_up") == 0)
  {
    std::thread(shutterUp, shutter).detach();
    publishShutterState(shutter, "up");
  }
  else if (strcmp(op, "shutter_down") == 0)
  {
    std::thread(shutterDown, shutter).detach();
    publishShutterState(shutter, "down");
  }
  else
  {
    Serial.print("received unknown operation: ");
    Serial.println(op);
  }

  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  Serial.println();

  setupPins();

  mqttClient.setServer(MQTT_SERVER_DOMAIN, MQTT_SERVER_PORT);
  mqttClient.setCallback(onMQTTMessage);

  Serial.println("\nready");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWifi();
    return;
  }

  if (!mqttClient.loop())
  {
    connectMQTT();
    return;
  }
}
