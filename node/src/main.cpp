#include <Arduino.h>
#include <atomic>
#include "freertos/timers.h"
#include "LoRa.h"

static const uint8_t greenPin = 12;//io34
static const uint8_t yellowPin = 13;//io14
static const uint8_t redPin = 15;//io15
static const uint8_t fridgePin = 2; //io22

static uint16_t yellowLightOnTimeMs = 2000;

static std::atomic<bool> hasBeenGreen;
TimerHandle_t yellowOnTimer = nullptr;
static void OnFridgePinChange();
static bool CheckFridgeIsOpen();
static bool CheckFridgeIsClosed();
static void SetStoplight(bool red, bool yellow, bool green);
static void SetYellowTimer();
static void OnYellowTimerTimeout(TimerHandle_t /*dummy*/);

//attachInterrupt(digitalPinToInterrupt(fridgePin));


void setup() {
  hasBeenGreen = false;
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin,OUTPUT);
  pinMode(fridgePin, INPUT);
  
  SetStoplight(true,false, false);

  yellowOnTimer = xTimerCreate("yellowOnTimer", pdMS_TO_TICKS(yellowLightOnTimeMs),pdTRUE,(void *)0,OnYellowTimerTimeout);
  attachInterrupt(digitalPinToInterrupt(fridgePin),OnFridgePinChange, CHANGE);
}

void loop() {
  // nothing
}

void OnFridgePinChange()
{
  if(CheckFridgeIsOpen())
  {
    SetStoplight(false, false, true);
  }
  else if(CheckFridgeIsClosed())
  {
    SetStoplight(false, true, false);
    SetYellowTimer();
  }
  else
  {
    //SetStoplight(true, false, false);
  }
}

bool CheckFridgeIsOpen()
{
  xTimerStop(yellowOnTimer, 0);
  bool ret = true;
  for(int i = 0; i < 100; i++)
  {
    if(digitalRead(fridgePin) == LOW)
    {
      ret = false;
    }
  }
  if(ret)
  {
    hasBeenGreen = true;
  }
  return ret;
}

bool CheckFridgeIsClosed()
{
  bool ret = false;
  if(hasBeenGreen && (digitalRead(fridgePin) == LOW))
  {
    ret = true;
    hasBeenGreen = false;
  }
  return ret;
}

void SetStoplight(bool red, bool yellow, bool green)
{
  //false == LOW and true == HIGH
  digitalWrite(redPin, !red);
  digitalWrite(yellowPin, yellow);
  digitalWrite(greenPin, green);
}

void SetYellowTimer()
{
  xTimerStart(yellowOnTimer, 0);
}

static void OnYellowTimerTimeout(TimerHandle_t /*dummy*/)
{
  SetStoplight(true, false, false);
}