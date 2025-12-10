#include "pins.h"
#include <Arduino.h>
#include "encoder.h"
#include "alarm.h"

static bool buzzing = false;

void ring_alarm(){
  if(!buzzing){
    tone(BUZZER_PIN, 262);
    buzzing = true;
  }
}

void stop_ring(){
  if(buzzing){
    noTone(BUZZER_PIN);
    buzzing = false;
  }
}

bool is_ringing(){
  return buzzing;
}

Alarm tempAlarm;

std::vector<Alarm> alarms;