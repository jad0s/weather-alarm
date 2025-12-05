#include "pins.h"
#include <Arduino.h>
#include "encoder.h"
#include "alarm.h"

void ring_alarm(){
  static bool buzzing = false;
  if(!buzzing){
    tone(BUZZER_PIN, 262);
    buzzing = true;
  }  
}

Alarm tempAlarm;

std::vector<Alarm> alarms;