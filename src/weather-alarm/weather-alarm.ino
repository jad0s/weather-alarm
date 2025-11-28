#include "display.h"
#include "encoder.h"
#include "touch.h"
#include "wifi.h"
#include "alarm.h"
#include "buzzer.h"
#include <Arduino.h>
#include "pins.h"
#include "weather.h"
#include "systemtime.h"
#include "ui.h"
#include "icons.h"

//loading animation. Every 200ms, a new frame is loaded, while loading is true.

void animationTask(void * parameter){ 
  int phase = 0;
  while(loading){
    display.clearDisplay();
    switch(phase){
      case 0: display.drawBitmap(0,0,loading_0,128,64,1); break;
      case 1: display.drawBitmap(0,0,loading_1,128,64,1); break;
      case 2: display.drawBitmap(0,0,loading_2,128,64,1); break;
      case 3: display.drawBitmap(0,0,loading_3,128,64,1); break;
    }
    display.display();
    phase = (phase + 1) % 4;
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  display.clearDisplay();
  display.display();

  vTaskDelete(NULL);
}

void setup() {
  initDisplay();
  xTaskCreate(animationTask, "animTask", 2048, NULL, 1, NULL); //first thing on boot is showing the animation.
  Serial.begin(115200);
  Alarm defaultAlarm;
  defaultAlarm.hour = 7;
  defaultAlarm.minute = 0;
  defaultAlarm.tempCond = COND_OFF;
  defaultAlarm.tempValue = 0;
  defaultAlarm.enabled = true;
  alarms.push_back(defaultAlarm);
  initWiFi();
  int WiFiMillis = millis();
  initTime();
  updateTime();  
  initBuzzer();
  encoder.begin();
  fetchWeather();
  updateDisplayTime();
  screenDirty = true;
  loading = false; //after all loading is done, terminate the animation task.
  display.clearDisplay();
  display.display();
  displayTouched();
}

void loop() {
  encoder.tick();
  int32_t steps = encoder.getSteps();
  bool clicked = encoder.wasClicked();
  bool touched = (digitalRead(TOUCH_PIN) == HIGH);

  hours = (getHours() + 1) % 24;
  minutes = getMinutes();

  static int alarmField = 0;

  // if the current screen is the homescreen and encoder is clicked, switch to the alarm screen
  if (clicked && screenOn == true) {
    Serial.println("Click");
    switch(uiState){
      case UI_HOME:
        uiState = UI_ALARM_LIST;
        screenDirty = true;
        clicked = false; //consume the click as it's no longer needed in this loop
        break;

    }
  }

  if(touched || clicked || steps != 0){
    displayTouched();
  }

    if (uiState == UI_ALARM_EDIT) {
      if (clicked) {
          if (editIndex == -1) {
              // Not currently editing a field: handle selection clicks (Delete/Back/enter edit)
              if (cursorIndex == 3) {           // Delete
                alarms.erase(alarms.begin() + selectedAlarmIndex);
                // if list empty, go back to ALARM_LIST
                if (alarms.empty()){
                  uiState = UI_ALARM_LIST;
                  cursorIndex = 0;
                  selectedAlarmIndex = 0;
                } else {
                  // clamp selection
                  if (selectedAlarmIndex >= (int)alarms.size()) selectedAlarmIndex = alarms.size()-1;
                  cursorIndex = 0;
                }
                editIndex = -1;
                screenDirty = true;
              }
              else if (cursorIndex == 4) {      // Back
                  uiState = UI_HOME;
                  cursorIndex = 0;
                  editIndex = -1;
                  screenDirty = true;
              }
              else {
                  // Enter edit mode for selected field (hour / cond / value / weather)
                  editIndex = cursorIndex;
              }
          } else {
              // Move to next sub-field
            subIndex++;

            // Determine number of subfields
            int count = 1;
            if (editIndex == 0) count = 2;    // time: hour + minute
            if (editIndex == 1 && alarms[selectedAlarmIndex].tempCond != COND_OFF)
                count = 2;                    // temp: cond + value

            if (subIndex >= count) {
                // exit edit mode
                editIndex = -1;
                subIndex = 0;
            }
          }
          screenDirty = true;
      }

    if (steps != 0) {
        if (editIndex == -1) {
            // moving the cursor while not editing
            cursorIndex = (cursorIndex + steps) % 4;
        } else {
            // editing the selected field on the selected alarm
            editAlarmField(alarms[selectedAlarmIndex], editIndex, subIndex, steps);
        }
        screenDirty = true;
    }
  }

  if (uiState == UI_ALARM_LIST) {

    if (steps != 0) {
        cursorIndex = constrain(cursorIndex + steps, 0, (int)alarms.size());
        screenDirty = true;
    }

    if (clicked) {
        if (cursorIndex == 0) {
            // "+ Add alarm"
            Alarm a;
            a.hour = 7;
            a.minute = 0;
            a.tempCond = COND_OFF;
            a.tempValue = 0;
            a.enabled = true;
            alarms.push_back(a);

            selectedAlarmIndex = alarms.size()-1;
            cursorIndex = 0;
            editIndex = -1;
            uiState = UI_ALARM_EDIT;
        } else {
            // select existing alarm
            selectedAlarmIndex = cursorIndex - 1;
            cursorIndex = 0;
            editIndex = -1;
            uiState = UI_ALARM_EDIT;
        }
        screenDirty = true;
    }
  }

  if(screenDirty){ //update the UI
    switch(uiState){
      case UI_HOME:
        updateDisplayTime();
        break;
      case UI_ALARM_LIST:
        drawAlarmList();
        break;
      case UI_ALARM_EDIT:
        drawAlarmEdit();
        break;
    }
    screenDirty = false;
  }


  if(millis()%60000 == 0){
    updateTime();  // update NTP time
  }

  //fetch new weather every 30 minutes
  //Open-Meteo updates every hour
  if (millis() - lastFetch > 30 * 60 * 1000){
    weatherValid = false;
    fetchWeather();
  }
    

  displayUpdate(); //check if last interaction is longer than screen timeout (10s)
  //if it is, turn screen off

  //check if any alarm should be activated
  for(int i = 0; i < alarms.size(); i++){
    Alarm &alarm = alarms[i]
    if(hours == alarm.hour && minutes == alarm.minute && alarm.rang == false){
      //check if temperature condition is met
      switch(alarm.tempCondition){
        case 0:
          alarm.active = true;
          break;
        case 1:
          if(cachedTemp < alarm.tempValue){
            alarm.active = true;
          }
          break;
        case 2:
          if(cachedTemp > alarm.tempValue){
            alarm.active = true;
          }
      }
    }

    if(alarm.active){
      alarm(); //activate buzzer
      alarm.rang = true;
      Serial.printf("alarm time is %d:%d, current time is %d:%d, alarming\n", alarmHour, alarmMinute, hours, minutes);
      if(digitalRead(TOUCH_PIN) == HIGH){
        alarm.active = false;
        noTone(BUZZER_PIN);
      }
    }
  }


  if(hours == 0 && minutes == 0){ //reset all alarms at midnight
    for(int i = 0; i < alarms.size(); i++){
      alarms[i].rang = false;
    }
  }

    

    
}
