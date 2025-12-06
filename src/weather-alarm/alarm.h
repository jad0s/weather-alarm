#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>
#include <vector>
#include "weather.h"

void ring_alarm();
void setAlarm();

enum TempCondition{
  COND_OFF = 0, //OFF
  COND_LT = 1, //Less than
  COND_GT = 2 //Greater than
};


struct Alarm{
  int hour = 7;
  int minute = 0;
  TempCondition tempCond = COND_OFF; //0 = OFF, 1 = < (less than), 2 = > (greater than)
  int tempValue = 0;
  bool enabled = true;
  bool rang = false;
  bool active = false;
  std::vector<WeatherCode> positive; //OR group
  std::vector<WeatherCode> negative; // AND group
};

extern Alarm tempAlarm;
extern std::vector<Alarm> alarms;

#endif
