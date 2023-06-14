#ifndef LORAMESSAGEPARSER_H
#define LORAMESSAGEPARSER_H
#include <Arduino.h>
#include "Stoplight.h"

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
  stopLightMode_e mode = AUTOMATIC;
  colorData_t color;
}unpackedMessage_t;

unpackedMessage_t ParseLoraMessage(uint8_t loraData);


#endif //LORAMESSAGEPARSER_H