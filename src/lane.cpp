#ifndef LANE_H

#include <Arduino.h>
#include "lane.h"
#include "config.h"

LaneTimer::LaneTimer(uint8_t pin)
{
  this->pin = pin;
  this->laps = 0;
  this->bestTime = 0;
  this->roundStartedAt = 0;
}

void LaneTimer::reset()
{
  this->laps = 0;
  this->bestTime = 0;
  this->roundStartedAt = 0;
}

ICACHE_RAM_ATTR void LaneTimer::interruptHandler()
{
  if (this->haveData)
  {
    Serial.println("Previous interrupt not consumed.");
    this->debounceMillis = millis();
    return;
  }

  int deltaT = millis() - debounceMillis;
  if (deltaT < DEBOUNCE_TIME)
  {
    Serial.printf("Debouncer ignored interrupt w/ [%dms].\n", deltaT);
    this->debounceMillis = millis();
    return;
  }

  Serial.printf("Interrupt deltaT [%dms]\n", deltaT);
  this->latestMillis = millis();
  this->debounceMillis = latestMillis;
  this->haveData = true;
}

void LaneTimer::registerLap(unsigned int time)
{
  if (this->laps > -1)
  {
    unsigned int lapTime = (time - this->roundStartedAt);
    if (this->bestTime == 0 || lapTime < this->bestTime)
    {
      this->bestTime = lapTime;
    }
  }

  this->laps++;
  this->roundStartedAt = time;
}

void LaneTimer::loop()
{
  if (this->haveData)
  {
    unsigned int triggerMillis = this->latestMillis;
    this->latestMillis = 0;
    this->haveData = false;

    this->registerLap(triggerMillis);
  }
}

String LaneTimer::shortStats()
{
  static char *segment1 = (char *)malloc(5);
  static char *segment2 = (char *)malloc(5);
  static char *segment3 = (char *)malloc(5);

  if (this->laps > 0)
  {
    sprintf(segment1, "%02d", this->laps);
  }
  else
  {
    strncpy(segment1, "  ", 3);
  }

  if (this->roundStartedAt > 0)
  {
    sprintf(segment2, "%0.1f", (millis() - (int)roundStartedAt) / 1000.0);
  }
  else
  {
    strncpy(segment2, "-.-", 4);
  }

  if (this->bestTime > 0)
  {
    sprintf(segment3, "%0.1f", this->bestTime / 1000.0);
  }
  else
  {
    strncpy(segment3, "-.-", 4);
  }

  char *stats = (char *)malloc(20);
  sprintf(stats, "L%s T%ss B%ss  ",
          segment1,
          segment2,
          segment3);

  return String(stats);
}

#endif
