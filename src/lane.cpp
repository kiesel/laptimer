#ifndef LANE_H

#include <Arduino.h>
#include "lane.h"
#include "config.h"

LaneTimer::LaneTimer(uint8_t pin)
{
  this->pin = pin;
  this->laps = -1;
  this->bestLap = 0;
  this->lastLap = 0;
  this->roundStartedAt = 0;
}

void LaneTimer::reset()
{
  this->laps = -1;
  this->bestLap = 0;
  this->lastLap = 0;
  this->roundStartedAt = 0;
}

void LaneTimer::interruptHandler()
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

    // Update best time if no time available, or better time
    if (this->bestLap == 0 || lapTime < this->bestLap)
    {
      this->bestLap = lapTime;
    }

    // Always store last lap
    this->lastLap = lapTime;
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
  static char *lapCount = (char *)malloc(5);
  static char *thisLapTime = (char *)malloc(5);
  static char *bestLapTime = (char *)malloc(5);
  static char *lastLapTime = (char *)malloc(5);

  if (this->laps >= 0)
  {
    sprintf(lapCount, "%02d", this->laps);
  }
  else
  {
    strncpy(lapCount, "  ", 3);
  }

  if (this->roundStartedAt > 0)
  {
    sprintf(thisLapTime, "%0.1f", (millis() - (int)roundStartedAt) / 1000.0);
  }
  else
  {
    strncpy(thisLapTime, "-.-", 4);
  }

  if (this->bestLap > 0)
  {
    sprintf(bestLapTime, "%0.1f", this->bestLap / 1000.0);
  }
  else
  {
    strncpy(bestLapTime, "-.-", 4);
  }

  if (this->lastLap > 0)
  {
    sprintf(lastLapTime, "%0.1f", this->lastLap / 1000.0);
  }
  else
  {
    strncpy(lastLapTime, "-.-", 4);
  }

  char *stats = (char *)malloc(20);
  sprintf(stats, "%s %s %s %s  ",
          lapCount,
          bestLapTime,
          lastLapTime,
          thisLapTime);

  return String(stats);
}

#endif
