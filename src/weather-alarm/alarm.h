#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>
#include <vector>
#include "weather.h"

void ring_alarm();
void setAlarm();
void stop_ring();
bool is_ringing();

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
  // Buddy alarm: optional simple time-only alarm that rings if this alarm did NOT ring.
  bool buddyEnabled = false;
  int buddyHour = 0;
  int buddyMinute = 0;
  bool buddyRang = false; // track if buddy has rung for the day
};

extern Alarm tempAlarm;
extern std::vector<Alarm> alarms;

#endif
