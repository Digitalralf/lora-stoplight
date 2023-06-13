#include <Arduino.h>
#include <atomic>
#include "freertos/timers.h"

typedef enum
{
    AUTOMATIC,
    MANUAL,
    FLICKER,
} stopLightMode_e;



class Stoplight
{
private:
    stopLightMode_e mode = AUTOMATIC;
    uint8_t green = 0;
    uint8_t yellow = 0;
    uint8_t red = 0;
    uint8_t fridge = 0;

    bool redOn = false;
    bool yellowOn = false;
    bool greenOn = false;
    uint16_t flickerIntervalMs = UINT16_MAX;

    std::atomic<bool> hasBeenGreen;
    TimerHandle_t yellowOnTimer = nullptr;


    void SetStopLightHardware();
    bool CheckFridgeIsOpen();
    bool CheckFridgeIsClosed();
    void SetMode(stopLightMode_e inputMode);
    void StopFunctionality();


public:
    Stoplight(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin, uint8_t fridgePin, stopLightMode_e modeSet = AUTOMATIC);
    ~Stoplight();

    void SetModeAutomatic();
    void SetModeManual(bool red, bool yellow, bool green);
    void SetModeFlicker(bool red, bool yellow, bool green, uint16_t flickerIntervalMs);
};
