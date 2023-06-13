#include "secrets.h"
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncMqttClient.hpp>
#include <RingBuf.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "SSD1306.h"
#include "LoRa.h"

#define OLED_ADDRESS    0x3C
#define OLED_SDA    21
#define OLED_SCL    22
#define OLED_RST    -1

#define CONFIG_MOSI 27
#define CONFIG_MISO 19
#define CONFIG_CLK  5
#define CONFIG_NSS  18
#define CONFIG_RST  23
#define CONFIG_DIO0 26

#define LORA_FREQUENCY 868

#define MQTT_HOST IPAddress(192, 168, 1, 2)
#define MQTT_PORT 1883

typedef enum
{
  AUTOMATIC,
  MANUAL,
  FLICKER
} messageType_e;

typedef enum
{
  ON,
  OFF
} state_e;

typedef struct
{
  bool red = false;
  bool yellow = false;
  bool green = false;
}colorData_t;
typedef struct
{
  state_e state = OFF;
  messageType_e mode = AUTOMATIC;
  colorData_t color;
}mqttMessage_t;

RingBuf<mqttMessage_t, 100>  mqttMessageBuffer;

SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
WiFiClient espClient;
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.setCredentials(username,passwd);
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("stoplight/status", 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(String topic, String payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.print("  payload: ");
  Serial.println(payload);
  
  DynamicJsonDocument doc(100);
  deserializeJson(doc, payload);
  mqttMessage_t inComingMessage;
  if(doc["state"] == "on")
  {
    Serial.println("State: ON");
    inComingMessage.state = ON;
  }
  if(doc["state"] == "off")
  {
    Serial.println("State: OFF");
    inComingMessage.state = OFF;
  }
  if(doc["mode"] == "Manual")
  {
    Serial.println("Mode: MANUAL");
    inComingMessage.mode = MANUAL;
  }
  if(doc["mode"] == "Flicker")
  {
    Serial.println("Mode: Flicker");
    inComingMessage.mode = FLICKER;
  }
  if(doc["mode"] == "Automatic")
  {
    Serial.println("Mode: Automatic");
    inComingMessage.mode = AUTOMATIC;
  }
  inComingMessage.color.green = doc["green"];
  Serial.print("Green: ");
  Serial.println(inComingMessage.color.green);
  inComingMessage.color.red = doc["red"];
  Serial.print("Red: ");
  Serial.println(inComingMessage.color.red);
  inComingMessage.color.yellow = doc["yellow"];
  Serial.print("Yellow: ");
  Serial.println(inComingMessage.color.yellow);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void loop() {
}