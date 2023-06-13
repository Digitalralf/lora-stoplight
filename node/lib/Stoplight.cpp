#include "Stoplight.h"

Stoplight::Stoplight(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin, uint8_t fridgePin, stopLightMode_e modeSet) : green(greenPin), red(redPin), yellow(yellowPin), fridge(fridgePin), mode(modeSet)
{
    hasBeenGreen = false;
    pinMode(greenPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(redPin,OUTPUT);
    pinMode(fridgePin, INPUT);

    
}

Stoplight::~Stoplight()
{
}