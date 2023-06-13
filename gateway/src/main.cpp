#include "secrets.h"
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncMqttClient.hpp>
#include <RingBuf.h>
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

typedef struct
{
  bool red = 0;
  bool yellow = 0;
  bool green = 0;
}colorData_t;
typedef struct
{
  bool toggle = false;
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
  uint16_t packetIdSub = mqttClient.subscribe("stoplight/switch", 0);
  uint16_t packetIdSub2 = mqttClient.subscribe("stoplight/rgb/set", 0);
  uint16_t packetIdSub3 = mqttClient.subscribe("stoplight/effect/set", 0);
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

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
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

  mqttMessage_t inComingMessage;
  String payloadFormat(payload);
  // for(int i = 0; i++; i < len)
  // {
  //   payloadFormat +=  payload[i];
  // }
  Serial.println(payloadFormat);

  if(payloadFormat == "TOGGLE")
  {
    Serial.println("Found Toggle");
    inComingMessage.toggle = true;
    mqttMessageBuffer.push(inComingMessage);
  }
  if(payloadFormat == "Manual")
  {
    Serial.println("Found Manual");
    inComingMessage.mode = MANUAL;
    mqttMessageBuffer.push(inComingMessage);
  }
  if(payloadFormat == "Automatic")
  {
    Serial.println("Found Automatic");
    inComingMessage.mode = AUTOMATIC;
    mqttMessageBuffer.push(inComingMessage);
  }
  if(payloadFormat == "Flicker")
  {
    Serial.println("Found Flicker");
    inComingMessage.mode = FLICKER;
    mqttMessageBuffer.push(inComingMessage);
  }

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









// void setup() 
// {
//   display.init();
//   display.flipScreenVertically();
//   display.clear();
//   display.setFont(ArialMT_Plain_16);
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(display.getWidth() / 2, display.getHeight() / 2,"Starting");
//   display.display();

//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//   while (WiFi.status() != WL_CONNECTED) 
//   {
//     delay(500);
//     display.clear();
//     display.setFont(ArialMT_Plain_16);
//     display.setTextAlignment(TEXT_ALIGN_CENTER);
//     display.drawString(display.getWidth() / 2, display.getHeight() / 2,"Connecting");
//     display.display();
//   }

//   display.clear();
//   display.setFont(ArialMT_Plain_16);
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(display.getWidth() / 2, display.getHeight() / 2, WiFi.localIP().toString());
//   display.display();

//   mqttClient.setCredentials(username, passwd);
//   mqttClient.connect();

// }

// void loop() {
//   // put your main code here, to run repeatedly:
// }