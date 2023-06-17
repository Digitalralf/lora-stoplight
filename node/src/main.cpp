#include <Arduino.h>
#include <atomic>
#include "freertos/timers.h"
#include "SSD1306.h"
#include "LoRa.h"
#include "Stoplight.h"
#include "LoraMessageParser.h"

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
SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
static const uint8_t greenPin = 12;//io34
static const uint8_t yellowPin = 13;//io14
static const uint8_t redPin = 15;//io15
static const uint8_t fridgePin = 2; //io2
static const uint16_t transmitIntervalMs = 2000;
static unsigned long lastSendTime = 0;
static void PrintToScreen(String string);
static void CheckForPacket();
static void TransmitFridgeOpened();


void setup() 
{
  Serial.begin(115200);
  StartStoplight(greenPin,yellowPin,redPin,fridgePin);
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);
    LoRa.setPins(CONFIG_NSS, CONFIG_RST, CONFIG_DIO0);
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
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
    TransmitFridgeOpened();
    lastSendTime = millis();            // timestamp the message
  }
  CheckForPacket();
}

void CheckForPacket()
{
  if (LoRa.parsePacket()) 
  {
    unpackedMessage_t unpackedMessage;
    uint8_t message = 0;
    while (LoRa.available()) 
    {
      message = (uint8_t)LoRa.read();
      Serial.println(message, BIN);
      PrintToScreen(String(LoRa.packetRssi()));
      unpackedMessage = ParseLoraMessage(message);
    }
    Serial.print("State: ");
    if(unpackedMessage.state == ON)
    {
      Serial.println("ON");
    }
    if(unpackedMessage.state == OFF)
    {
      Serial.println("OFF");
    }

    Serial.print("Mode: ");
    if(unpackedMessage.mode == AUTOMATIC)
    {
      Serial.println("Automatic");
      SetStopLightAutomatic();
    }
    if(unpackedMessage.mode == FLICKER)
    {
      Serial.println("Flicker");
      SetStopLightFlicker(unpackedMessage.color.red, unpackedMessage.color.yellow, unpackedMessage.color.green, 1000);
    }
    if(unpackedMessage.mode == MANUAL)
    {
      SetStopLightColors(unpackedMessage.color.red, unpackedMessage.color.yellow, unpackedMessage.color.green);
      Serial.println("Manual");
    }

    Serial.print("Red: ");
    Serial.println(unpackedMessage.color.red);
    Serial.print("Green: ");
    Serial.println(unpackedMessage.color.green);
    Serial.print("Yellow: ");
    Serial.println(unpackedMessage.color.yellow);
  }
}

void TransmitFridgeOpened()
{
  static uint8_t ping = 0b10101010;
  if(CheckIfFridgeOpened())
  {
    Serial.println(ping, BIN);
    LoRa.beginPacket();
    LoRa.write(ping);
    LoRa.endPacket();
  }
}

void PrintToScreen(String string)
{
  display.clear();
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, string.c_str());
  display.display();
}