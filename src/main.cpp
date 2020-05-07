#include <thread>

#include <esp_err.h>
#include <esp_pthread.h>

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
    if (shutter < total)
      return &c;
  }

  throw std::invalid_argument("no such shutter");
}

#define StaticMQTTJsonDocument StaticJsonDocument<256>

chrono_ms double_seconds_to_chrono_ms(double secs)
{
  return chrono_ms((int)(1000 * secs));
}

void handle_command(StaticMQTTJsonDocument doc)
{
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

  if (strcmp(op, "shutter_stop") == 0)
  {
    controller->roll_stop(shutter);
    return;
  }

  bool roll_up;
  if (strcmp(op, "shutter_up") == 0)
    roll_up = true;
  else if (strcmp(op, "shutter_down") == 0)
    roll_up = false;
  else
  {
    Serial.print("received unknown operation: ");
    Serial.println(op);
    return;
  }

  const char *mode = doc["mode"] | "default";
  if (strcmp(mode, "absolute") == 0)
  {
    const auto time = double_seconds_to_chrono_ms(doc["time"]);
    const auto total_time = double_seconds_to_chrono_ms(doc["total_time"] | DEFAULT_TOTAL_TIME);
    const shutter::ShutterProfile profile{shutter, total_time};

    if (roll_up)
      controller->roll_from_bottom(profile, time);
    else
      controller->roll_from_top(profile, time);
  }
  else if (strcmp(mode, "relative") == 0)
  {
    const auto time = double_seconds_to_chrono_ms(doc["time"] | DEFAULT_RELATIVE_TIME);

    if (roll_up)
      controller->roll_up(shutter, time);
    else
      controller->roll_down(shutter, time);
  }
  else
  {
    if (roll_up)
      controller->roll_up(shutter);
    else
      controller->roll_down(shutter);
  }
}

void on_mqtt_message(char *topic, byte *payload, unsigned int length)
{
  StaticMQTTJsonDocument doc;
  const auto err = deserializeJson(doc, payload, length);
  if (err)
  {
    Serial.print("failed to deserialize message: ");
    Serial.println(err.c_str());
    return;
  }

  std::thread(handle_command, doc).detach();
}

void panic(std::string msg)
{
  Serial.print("PANIC: ");
  Serial.println(msg.c_str());
  abort();
}

void set_thread_config()
{
  esp_pthread_cfg_t cfg;
  cfg.stack_size = (4 * 1024);
  cfg.inherit_cfg = true;
  if (esp_pthread_set_cfg(&cfg) != ESP_OK)
  {
    panic("failed to set pthread config");
  }
}

void setup()
{
  for (auto &c : CONTROLLERS)
  {
    c.setup();
  }

  Serial.begin(9600);
  Serial.println();

  set_thread_config();

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
