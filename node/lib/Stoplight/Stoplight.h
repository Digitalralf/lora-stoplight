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



// class Stoplight
// {
// private:
//     stopLightMode_e _mode = AUTOMATIC;
//     uint8_t _greenPin = 0;
//     uint8_t _yellowPin = 0;
//     uint8_t _redPin = 0;
//     uint8_t _fridgePin = 0;

//     bool _redOn = false;
//     bool _yellowOn = false;
//     bool _greenOn = false;
//     uint16_t flickerIntervalMs = UINT16_MAX;

//     std::atomic<bool> _hasBeenGreen;
//     TimerHandle_t _greenTimer = nullptr;
//     TimerHandle_t _yellowTimer = nullptr;
//     TimerHandle_t _redTimer = nullptr;

//     uint16_t _timeUntilRedLightOnMs = 2000;

//     void InitHardware();
//     void SetStopLightHardware(bool red, bool yellow, bool green);
//     bool CheckFridgeIsOpen();
//     bool CheckFridgeIsClosed();
//     void SetMode(stopLightMode_e inputMode);
//     void StopFunctionality();
//     void AttachInterruptFridgeOpening();
//     void AttachInterruptFridgeClosing();
//     void DeAttachInterrupt();
//     void InitGreenTimer();
//     void InitYellowTimer();
//     void InitRedTimer();

//     //Make them static only one can be instatiated for now
//     void StartGreenTimer();
//     void StartYellowTimer();
//     void StartRedTimer();

//     void OnGreenTimerTimeout(TimerHandle_t /*dummy*/);
//     void OnYellowTimerTimeout(TimerHandle_t /*dummy*/);
//     void OnRedTimerTimeout(TimerHandle_t /*dummy*/);

// public:
//     Stoplight(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin, uint8_t fridgePin, stopLightMode_e modeSet = AUTOMATIC);
//     ~Stoplight();

//     void SetModeAutomatic();
//     void SetModeManual(bool red, bool yellow, bool green);
//     void SetModeFlicker(bool red, bool yellow, bool green, uint16_t flickerIntervalMs);
// };
