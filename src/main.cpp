#include <thread>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <config.hpp>

WiFiClient g_wifi_client;
PubSubClient g_mqtt_client(g_wifi_client);

void connect_wifi()
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

void mqtt_subscribe()
{
  Serial.println("Subscribing to MQTT topics");
  g_mqtt_client.subscribe("ewfs/command");
}

void connect_mqtt()
{
  Serial.println("Connecting to MQTT");
  if (!g_mqtt_client.connect(MQTT_CLIENT_ID))
    return;

  mqtt_subscribe();
}

void publish_shutter_state(uint8_t shutter, String state)
{
  StaticJsonDocument<256> doc;
  doc["assumed_state"] = state;

  char buf[256];
  auto n = serializeJson(doc, buf);

  char topicBuf[32];
  sprintf(topicBuf, "ewfs/shutters/%u", shutter);
  g_mqtt_client.publish(topicBuf, buf, n);
}

shutter::Controller *get_controller(ShutterIndex shutter)
{
  uint total = 0;
  for (auto &c : CONTROLLERS)
  {
    total += c.total_shutters();
    if (shutter > total)
      return &c;
  }

  throw std::invalid_argument("no such shutter");
}

void on_mqtt_message(char *topic, byte *payload, unsigned int length)
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

  shutter::Controller *controller;
  try
  {
    controller = get_controller(shutter);
  }
  catch (const std::invalid_argument &e)
  {
    Serial.print("invalid shutter: ");
    Serial.println(shutter);
    return;
  }

  if (strcmp(op, "shutter_up") == 0)
  {
    controller->roll_up(shutter);
    publish_shutter_state(shutter, "up");
  }
  else if (strcmp(op, "shutter_down") == 0)
  {
    controller->roll_down(shutter);
    publish_shutter_state(shutter, "down");
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

  for (auto &c : CONTROLLERS)
  {
    c.setup();
  }

  g_mqtt_client.setServer(MQTT_SERVER_DOMAIN, MQTT_SERVER_PORT);
  g_mqtt_client.setCallback(on_mqtt_message);

  Serial.println("\nready");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connect_wifi();
    return;
  }

  if (!g_mqtt_client.loop())
  {
    connect_mqtt();
    return;
  }
}
