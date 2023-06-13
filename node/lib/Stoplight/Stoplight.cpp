#include "Stoplight.h"

// static void OnGreenTimerTimeout(TimerHandle_t /*dummy*/);
// static void OnYellowTimerTimeout(TimerHandle_t /*dummy*/);
// static void OnRedTimerTimeout(TimerHandle_t /*dummy*/);

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
    uint16_t flickerIntervalMs = UINT16_MAX;


    uint16_t _timeUntilRedLightOnMs = 2000;
    std::atomic<bool> _hasBeenGreen;
    TimerHandle_t _greenTimer = nullptr;
    TimerHandle_t _yellowTimer = nullptr;
    TimerHandle_t _redTimer = nullptr;
} stopLightData_t;

static stopLightData_t stopLightData;

static void InitHardware();
static void SetStopLightHardware(bool red, bool yellow, bool green);
static bool CheckFridgeIsOpen();
static bool CheckFridgeIsClosed();
static void AttachInterruptFridgeChange();
static void DeAttachInterrupt();
static void InitRedTimer();


static void OnFridgePinChange();
static void StartYellowTimer();
static void StartRedTimer();

static void OnGreenTimerTimeout(TimerHandle_t /*dummy*/);
static void OnYellowTimerTimeout(TimerHandle_t /*dummy*/);
static void OnRedTimerTimeout(TimerHandle_t /*dummy*/);


void StartStoplight(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin, uint8_t fridgePin, stopLightMode_e modeSet)
{
    stopLightData._mode = modeSet;
    stopLightData._hasBeenGreen = false;
    stopLightData._greenPin = greenPin;
    stopLightData._redPin = redPin;
    stopLightData._yellowPin = yellowPin;
    stopLightData._fridgePin = fridgePin;

    Serial.begin(115200);
    Serial.println("Hello Trying To Init");


    InitHardware();
}

void InitHardware()
{
    pinMode(stopLightData._greenPin, OUTPUT);
    pinMode(stopLightData._yellowPin, OUTPUT);
    pinMode(stopLightData._redPin,OUTPUT);
    pinMode(stopLightData._fridgePin, INPUT);

    SetStopLightHardware(true, false, false);

    AttachInterruptFridgeChange();;
    InitRedTimer();
}

void SetStopLightHardware(bool red, bool yellow, bool green)
{
    digitalWrite(stopLightData._greenPin, green);
    digitalWrite(stopLightData._yellowPin, yellow);
    //pin is inverted on hardware
    digitalWrite(stopLightData._redPin, !red);
}

void InitRedTimer()
{
    stopLightData._redTimer = xTimerCreate("redTimer", pdMS_TO_TICKS(2000),pdTRUE,(void *)0,OnRedTimerTimeout);
}

void AttachInterruptFridgeChange()
{
    attachInterrupt(digitalPinToInterrupt(stopLightData._fridgePin), OnFridgePinChange, CHANGE);
}

void DeAttachInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(stopLightData._fridgePin));
}

void OnFridgePinChange()
{
  if(CheckFridgeIsOpen())
  {
    SetStopLightHardware(false, false, true);
  }
  else if(CheckFridgeIsClosed())
  {
    SetStopLightHardware(false, true, false);
    StartRedTimer();
  }
  else
  {
    //SetStoplight(true, false, false);
  }
}

bool CheckFridgeIsOpen()
{
  xTimerStop(stopLightData._redTimer, 0);
  bool ret = true;
  for(int i = 0; i < 100; i++)
  {
    if(digitalRead(stopLightData._fridgePin) == LOW)
    {
      ret = false;
    }
  }
  if(ret)
  {
    stopLightData._hasBeenGreen = true;
  }
  return ret;
}

bool CheckFridgeIsClosed()
{
  bool ret = false;
  if(stopLightData._hasBeenGreen && (digitalRead(stopLightData._fridgePin) == LOW))
  {
    ret = true;
    stopLightData._hasBeenGreen = false;
  }
  return ret;
}

void StartRedTimer()
{
    xTimerStart(stopLightData._redTimer,0);
}

void OnRedTimerTimeout(TimerHandle_t /*dummy*/)
{
    SetStopLightHardware(true, false, false);
}






