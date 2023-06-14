#include "Stoplight.h"
typedef struct
{
    stopLightMode_e _mode = AUTOMATIC;
    uint8_t _greenPin = 0;
    uint8_t _yellowPin = 0;
    uint8_t _redPin = 0;
    uint8_t _fridgePin = 0;

    bool _redOn = false;
    bool _yellowOn = false;
    bool _greenOn = false;

    uint16_t _timeUntilRedLightOnMs = 2000;
    uint16_t _flickerTimeMs = UINT16_MAX;
    std::atomic<bool> _hasBeenGreen;
    TimerHandle_t _flickerTimer = nullptr;
    TimerHandle_t _redTimer = nullptr;
    TimerHandle_t _yellowTimer = nullptr;
    TimerHandle_t _greenTimer = nullptr;
} stopLightData_t;

static stopLightData_t stopLightData;

static void InitHardware();
static void SetStopLightHardware(bool red, bool yellow, bool green);
static void AttachInterruptFridgeOpening();
static void AttachInterruptFridgeClosing();
static void DeAttachInterrupt();

static void InitFlickerTimer();
static void StartFlickerTimer();
static void StopFlickerTimer();
static void OnFlickerTimeout(TimerHandle_t /*dummy*/);

static void InitGreenTimer();
static void StartGreenTimer();
static void StopGreenTimer();
static void OnGreenTimerTimeout(TimerHandle_t /*dummy*/);

static void InitYellowTimer();
static void StartYellowTimer();
static void StopYellowTimer();
static void OnYellowTimerTimeout(TimerHandle_t /*dummy*/);

static void InitRedTimer();
static void StartRedTimer();
static void StopRedTimer();
static void OnRedTimerTimeout(TimerHandle_t /*dummy*/);


void StartStoplight(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin, uint8_t fridgePin, stopLightMode_e modeSet)
{
    stopLightData._mode = modeSet;
    stopLightData._hasBeenGreen = false;
    stopLightData._greenPin = greenPin;
    stopLightData._redPin = redPin;
    stopLightData._yellowPin = yellowPin;
    stopLightData._fridgePin = fridgePin;

    InitHardware();
    
}

void SetStopLightAutomatic()
{
    StopFlickerTimer();
    StopRedTimer();
    stopLightData._greenOn = false;
    stopLightData._yellowOn = false;
    stopLightData._redOn = true;
    SetStopLightHardware(stopLightData._redOn,stopLightData._yellowOn,stopLightData._greenOn);
    stopLightData._mode = AUTOMATIC;
}
void SetStopLightColors(bool red, bool yellow, bool green)
{
    StopFlickerTimer();
    StopRedTimer();
    stopLightData._redOn = red;
    stopLightData._yellowOn = yellow;
    stopLightData._greenOn = green;
    SetStopLightHardware(stopLightData._redOn,stopLightData._yellowOn,stopLightData._greenOn);
    stopLightData._mode = MANUAL;
}
void SetStopLightFlicker(bool red, bool yellow, bool green, uint16_t intervalMs)
{
    StopFlickerTimer();
    StopRedTimer();

    stopLightData._flickerTimeMs = intervalMs;
    stopLightData._mode = FLICKER;
    stopLightData._redOn = red;
    stopLightData._yellowOn = yellow;
    stopLightData._greenOn = green;
    InitFlickerTimer();
    StartFlickerTimer();
}

void InitHardware()
{
    pinMode(stopLightData._greenPin, OUTPUT);
    pinMode(stopLightData._yellowPin, OUTPUT);
    pinMode(stopLightData._redPin,OUTPUT);
    pinMode(stopLightData._fridgePin, INPUT);

    SetStopLightHardware(true, false, false);
    InitRedTimer();
    InitGreenTimer();
    InitYellowTimer();
    InitFlickerTimer();

    AttachInterruptFridgeOpening();
}

void SetStopLightHardware(bool red, bool yellow, bool green)
{
    digitalWrite(stopLightData._greenPin, green);
    digitalWrite(stopLightData._yellowPin, yellow);
    //pin is inverted on hardware
    digitalWrite(stopLightData._redPin, !red);
}
void InitFlickerTimer()
{
    stopLightData._flickerTimer = xTimerCreate("redTimer", pdMS_TO_TICKS(stopLightData._flickerTimeMs),pdTRUE,(void *)0,OnFlickerTimeout);
}
void InitGreenTimer()
{
    stopLightData._greenTimer = xTimerCreate("redTimer", pdMS_TO_TICKS(20),pdFALSE,(void *)0,OnGreenTimerTimeout);
}
void InitYellowTimer()
{
    stopLightData._yellowTimer = xTimerCreate("redTimer", pdMS_TO_TICKS(20),pdFALSE,(void *)0,OnYellowTimerTimeout);
}
void InitRedTimer()
{
    stopLightData._redTimer = xTimerCreate("redTimer", pdMS_TO_TICKS(stopLightData._timeUntilRedLightOnMs),pdFALSE,(void *)0,OnRedTimerTimeout);
}

void AttachInterruptFridgeOpening()
{
    attachInterrupt(digitalPinToInterrupt(stopLightData._fridgePin), StartGreenTimer, RISING);
}

void AttachInterruptFridgeClosing()
{
    attachInterrupt(digitalPinToInterrupt(stopLightData._fridgePin), StartYellowTimer, FALLING);
}

void DeAttachInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(stopLightData._fridgePin));
}


void StartGreenTimer()
{
    detachInterrupt(digitalPinToInterrupt(stopLightData._fridgePin));
    xTimerStart(stopLightData._greenTimer, 0);
}
void OnGreenTimerTimeout(TimerHandle_t /*dummy*/)
{
    StopRedTimer();
    if(digitalRead(stopLightData._fridgePin) == HIGH)
    {
        if(stopLightData._mode == AUTOMATIC)
        {
            SetStopLightHardware(false, false, true);
        }
        AttachInterruptFridgeClosing();
    }
    else
    {
        AttachInterruptFridgeOpening();
    }
}

void StartYellowTimer()
{
    detachInterrupt(digitalPinToInterrupt(stopLightData._fridgePin));
    xTimerStart(stopLightData._yellowTimer, 0);
}

void OnYellowTimerTimeout(TimerHandle_t /*dummy*/)
{
    if(digitalRead(stopLightData._fridgePin) == LOW)
    {
        if(stopLightData._mode == AUTOMATIC)
        {
            SetStopLightHardware(false, true, false);
        }
        StartRedTimer();
        AttachInterruptFridgeOpening();
    }
    else
    {
        AttachInterruptFridgeClosing();
    }
}

void StartRedTimer()
{
    xTimerStart(stopLightData._redTimer,0);
}

void StopRedTimer()
{
    xTimerStop(stopLightData._redTimer,0);
}

void StartFlickerTimer()
{
    xTimerChangePeriod(stopLightData._flickerTimer, stopLightData._flickerTimeMs, 0);
    xTimerStart(stopLightData._flickerTimer, 0);
}

void StopFlickerTimer()
{
    xTimerStop(stopLightData._flickerTimer, 0);
}

void OnRedTimerTimeout(TimerHandle_t /*dummy*/)
{
    SetStopLightHardware(true, false, false);
}

void OnFlickerTimeout(TimerHandle_t /*dummy*/)
{
    static bool on = true;
    if(on)
    {
        SetStopLightHardware(stopLightData._redOn,stopLightData._yellowOn,stopLightData._greenOn);
    }
    else
    {
        SetStopLightHardware(false, false, false);
    }
    on = !on;
}







