#ifndef STOPLIGHT_H
#define STOPLIGHT_H

#include <Arduino.h>
#include <atomic>
#include "freertos/timers.h"

typedef enum
{
    AUTOMATIC,
    MANUAL,
    FLICKER,
} stopLightMode_e;

void StartStoplight(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin, uint8_t fridgePin, stopLightMode_e modeSet = AUTOMATIC);
void SetStopLightAutomatic();
void SetStopLightColors(bool red, bool yellow, bool green);
void SetStopLightFlicker(bool red, bool yellow, bool green, uint16_t intervalMs);

#endif //STOPLIGHT_H