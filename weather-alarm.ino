#include "display.h"
#include "encoder.h"
#include "touch.h"
#include "wifi.h"
#include "alarm.h"
#include "buzzer.h"
#include <Arduino.h>
#include "pins.h"
#include "http.h"



void setup() {
  initDisplay();
  displayLogo();
  Serial.begin(115200);
  initWiFi();
  updateTime();  
  initBuzzer();
  encoder.begin();
  display.clearDisplay();
  display.display();
  fetchWeather();
}

void loop() {
  encoder.tick();

  // read steps (signed) and print direction
  int32_t steps = encoder.getSteps();
  if (steps != 0) {
    // if steps magnitude >1, user turned quickly; print each detent
    while (steps > 0) { Serial.println("CW"); steps--; }
    while (steps < 0) { Serial.println("CCW"); steps++; }
  }

  // read button
  if (encoder.wasClicked()) {
    Serial.println("Click");
    fetchWeather();
  }
  if(millis()%60000 == 0){
    updateTime();
  }
          // update NTP time
  int h = (getHours() + 2) % 24;
  int m = getMinutes();
  int ah = 4;
  int am = 17;
    
    

  int touchState = digitalRead(TOUCH_PIN);
  if(touchState == HIGH){
    displayTouched();
  }

  displayUpdate();

    
  updateDisplayTime(h, m);

  if(!alarmActive && ah == h && am == m){
    alarmActive = true;
  }

  if(alarmActive){
    alarm();
    if(digitalRead(TOUCH_PIN) == HIGH){
      alarmActive = false;
      noTone(BUZZER_PIN);
    }
  }

    

    
}
