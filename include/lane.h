#define LANE_H

#include <Arduino.h>

class LaneTimer
{
private:
  uint8_t pin;
  int laps;
  unsigned int roundStartedAt;
  unsigned int bestLap;
  unsigned int lastLap;

  volatile bool haveData;
  volatile uint latestMillis;
  volatile uint debounceMillis;

public:
  LaneTimer(uint8_t pin);
  void registerLap(unsigned int time);
  void reset();
  void interruptHandler();
  void loop();
  String shortStats();
};
