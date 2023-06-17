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
#include "mqttLoraParser.h"

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

#define LORA_FREQUENCY 868E6

#define MQTT_HOST IPAddress(192, 168, 1, 2)
#define MQTT_PORT 1883

static const uint16_t transmitIntervalMs = 2000;
static unsigned long lastSendTime = 0;
static RingBuf<uint8_t, 100>  loraMessageBuffer;
static RingBuf<int, 10> loraRSSI;

SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
WiFiClient espClient;
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
//TimerHandle_t LoraTransmitTimer;


static void TransmitLora();
static void CheckForPacket();
static void PrintToScreen(String string);

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.setCredentials(username,password);
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        PrintToScreen("Wifi Connected");
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
  PrintToScreen("MQTT connected");
  //xTimerStart(LoraTransmitTimer, 0);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  //xTimerStop(LoraTransmitTimer, 0);
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

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
  String payloadClean(payload, len);
  Serial.println(payloadClean);
  
  uint8_t DataByte = ParseMqttToByte(payloadClean);
  Serial.print("DataByte: ");
  Serial.println(DataByte,BIN);
  loraMessageBuffer.push(DataByte);
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
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);

  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);
    LoRa.setPins(CONFIG_NSS, CONFIG_RST, CONFIG_DIO0);
    if (!LoRa.begin(LORA_FREQUENCY)) 
    {
        Serial.println("Starting LoRa failed!");
        while (1);
    }

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  //LoraTransmitTimer  = xTimerCreate("loraTimer", pdMS_TO_TICKS(2000), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(TransmitLora));


  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
  Serial.print("Current Spreading Factor: ");
  Serial.println(LoRa.getSpreadingFactor());
  Serial.print("Current Bandwith");
  Serial.println(LoRa.getSignalBandwidth());
  Serial.println("Starting Successful");
  PrintToScreen("Init Succesfull");

}

void loop() 
{
  if (millis() - lastSendTime > transmitIntervalMs) {
    TransmitLora();
    lastSendTime = millis();            // timestamp the message
  }
  CheckForPacket();
  // int rssi = 0;
  // if(loraRSSI.pop(rssi))
  // {
  //   String rssiString = String(rssi);
  //   PrintToScreen(rssiString);
  // }
}

void TransmitLora()
{
  uint8_t dataToSend = 0;
  if(loraMessageBuffer.pop(dataToSend))
  {
    Serial.println("Sending lora messsage");
    LoRa.beginPacket();
    LoRa.write(dataToSend);
    LoRa.endPacket();
    PrintToScreen(String(LoRa.packetRssi()));
    //loraRSSI.push(LoRa.rssi());
  }
}
void CheckForPacket()
{
  if (LoRa.parsePacket()) 
  {
    Serial.println("parsing packet");
    static const uint8_t expectedMessage = 0b10101010;
    static uint8_t message = 0;
    while (LoRa.available()) 
    {
      message = (uint8_t)LoRa.read();
      Serial.println(message, BIN);
      PrintToScreen(String(LoRa.packetRssi()));
      if(message == expectedMessage)
      {
        mqttClient.publish("koelkast/status",0,false,"ping",10);
      }
    }
  }
}
void PrintToScreen(String string)
{
  display.clear();
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, string.c_str());
  display.display();
}