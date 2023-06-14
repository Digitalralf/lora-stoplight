#include "mqttLoraParser.h"
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

mqttMessage_t parseToStructure(String jsonString);
uint8_t parseToByte(mqttMessage_t inComingMessage);


uint8_t ParseMqttToByte(String jsonString)
{
    mqttMessage_t inComingMessage = parseToStructure(jsonString);
    uint8_t loraMessage = parseToByte(inComingMessage);
    return loraMessage;
}

mqttMessage_t parseToStructure(String jsonString)
{   
    mqttMessage_t inComingMessage;
    DynamicJsonDocument doc(150);
    deserializeJson(doc, jsonString);
    if(doc["state"] == "on")
    {
        inComingMessage.state = ON;
    }
    if(doc["state"] == "off")
    {
        inComingMessage.state = OFF;
    }
    if(doc["mode"] == "Manual")
    {
        inComingMessage.mode = MANUAL;
    }
    if(doc["mode"] == "Flicker")
    {
        inComingMessage.mode = FLICKER;
    }
    if(doc["mode"] == "Automatic")
    {
        inComingMessage.mode = AUTOMATIC;
    }
    inComingMessage.color.green = doc["green"];
    inComingMessage.color.red = doc["red"];
    inComingMessage.color.yellow = doc["yellow"];
    Serial.println(inComingMessage.color.yellow);
    
    return inComingMessage;
}

uint8_t parseToByte(mqttMessage_t inComingMessage)
{
    const static uint8_t redBitMaskOn    = 0b00000001;
    const static uint8_t yellowBitMaskOn = 0b00000010;
    const static uint8_t greenBitMask    = 0b00000100;
    
    
    const static uint8_t AutomaticModeBitMaskOn     = 0b00000000;
    const static uint8_t ManualModeBitMaskOn        = 0b00001000;
    const static uint8_t FlickerModeBitMaskOn       = 0b00010000;

    const static uint8_t offMessage = ManualModeBitMaskOn;

    uint8_t DataByte = 0b00000000;
    if(inComingMessage.state == ON)
    {
        if(inComingMessage.color.green == true)
        {
            DataByte = DataByte | greenBitMask;
        }
        if(inComingMessage.color.red == true)
        {
            DataByte = DataByte | redBitMaskOn;
        }
        if(inComingMessage.color.yellow == true)
        {
            DataByte = DataByte | yellowBitMaskOn;
        }

        switch (inComingMessage.mode)
        {
            case AUTOMATIC:
                DataByte = AutomaticModeBitMaskOn;
                break;
            case MANUAL:
                DataByte = DataByte | ManualModeBitMaskOn;
                break;
            case FLICKER:
                DataByte = DataByte | FlickerModeBitMaskOn;
                break;
            default:
                DataByte = offMessage;
                break;
        }
    }
    else
    {
        DataByte = offMessage;
    }
    return DataByte;
}