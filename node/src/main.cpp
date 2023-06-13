#include <Arduino.h>
#include <atomic>
#include "freertos/timers.h"
#include "LoRa.h"
#include "Stoplight.h"

static const uint8_t greenPin = 12;//io34
static const uint8_t yellowPin = 13;//io14
static const uint8_t redPin = 15;//io15
static const uint8_t fridgePin = 2; //io2

void setup() 
{
  StartStoplight(greenPin,yellowPin,redPin,fridgePin);
}

void loop() 
{
  // nothing
}