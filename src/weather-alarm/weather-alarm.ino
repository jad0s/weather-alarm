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
  display.ssd1306_command(SSD1306_DISPLAYON);
  screenOn = true;
}

void loop() {
  encoder.tick();
  int32_t steps = encoder.getSteps();
  bool clicked = encoder.wasClicked();
  bool touched = (digitalRead(TOUCH_PIN) == HIGH);

  static bool prevTouched = false;
  bool touchPressed = (touched && !prevTouched);

  // If the buzzer is ringing and the user just touched, stop it immediately
  if (touchPressed) {
    if (is_ringing()) {
      stop_ring();
      screenDirty = true;
    }
  }

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
    if(!screenOn){
      touched = false;
      clicked = false;
      steps = 0;
    }
  }

    if (uiState == UI_ALARM_EDIT) {
      if (clicked) {
            if (editIndex == -1) {
              // Not currently editing a field: handle selection clicks (Back/enter edit)
              if (cursorIndex == 2) {
                  uiState = UI_WEATHER_MENU;
                  cursorIndex = 0;
                  editIndex = -1;
                  subIndex = 0;
                  clicked = false; // consume click so menu isn't immediately acted on
                  screenDirty = true;
              }

                else if (cursorIndex == 4) {      // Back (index shifted after removing Delete)
                  //save the temp alarm into real alarm
                  alarms[selectedAlarmIndex] = tempAlarm;

                  uiState = UI_HOME;
                  cursorIndex = 0;
                  editIndex = -1;
                  screenDirty = true;
              }
              else {
                  // Enter edit mode for selected field (hour / cond / value / weather)
                  editIndex = cursorIndex;
                  subIndex = 0; // start editing at first sub-field
                  // if entering buddy edit, enable buddy on the temp alarm so user can set time
                  if (editIndex == 3) tempAlarm.buddyEnabled = true;
              }
          } else {
              // Move to next sub-field
            subIndex++;

            // Determine number of subfields
            int count = 1;
            if (editIndex == 0) count = 2;    // time: hour + minute
            // use the temporary alarm being edited so changes to condition immediately expose the value subfield
            if (editIndex == 1 && tempAlarm.tempCond != COND_OFF)
              count = 2;                    // temp: cond + value
            // Buddy has two subfields (hour + minute)
            if (editIndex == 3) count = 2;

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
            // wrap within 0..4
            int maxIdx = 4;
            int r = (cursorIndex + steps) % (maxIdx + 1);
            if (r < 0) r += (maxIdx + 1);
            cursorIndex = r;
        } else {
            // editing the selected field on the selected alarm
            editAlarmField(editIndex, subIndex, steps);
        }
        screenDirty = true;
    }
  }

    // touch actions: detect rising edge
    if (touchPressed) {
      if (uiState == UI_ALARM_EDIT && editIndex == -1 && cursorIndex == 3) {
        // disable buddy instantly
        tempAlarm.buddyEnabled = false;
        screenDirty = true;
      }
      if (uiState == UI_ALARM_LIST && cursorIndex == 0) {
        // Touching the '+ Add alarm' entry should act like a Back/Home button
        uiState = UI_HOME;
        cursorIndex = 0;
        screenDirty = true;
      } else if (uiState == UI_ALARM_LIST && cursorIndex > 0) {
        // touch on an alarm in the list -> ask for confirmation to delete
        selectedAlarmIndex = cursorIndex - 1;
        uiState = UI_ALARM_DELETE_CONFIRM;
        cursorIndex = 0;
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
            //create a temporary alarm for the UI
            //this prevents the alarm from activating during it's setup
            tempAlarm = alarms[selectedAlarmIndex];
            uiState = UI_ALARM_EDIT;
        } else {
            // select existing alarm
            selectedAlarmIndex = cursorIndex - 1;
            cursorIndex = 0;
            editIndex = -1;
            //create a temporary alarm for the UI
            //this prevents the alarm from activating during it's setup
            tempAlarm = alarms[selectedAlarmIndex];
            uiState = UI_ALARM_EDIT;
        }
        screenDirty = true;
    }
  }

  if (uiState == UI_ALARM_DELETE_CONFIRM) {
    if (steps != 0) {
      cursorIndex = constrain(cursorIndex + steps, 0, 1);
      screenDirty = true;
    }

    if (clicked) {
      if (cursorIndex == 0) {
        // YES — delete selected alarm
        if (selectedAlarmIndex >= 0 && selectedAlarmIndex < alarms.size()) {
          alarms.erase(alarms.begin() + selectedAlarmIndex);
        }
        // clamp selection
        if (alarms.empty()) {
          uiState = UI_ALARM_LIST;
          cursorIndex = 0;
          selectedAlarmIndex = 0;
        } else {
          if (selectedAlarmIndex >= (int)alarms.size()) selectedAlarmIndex = alarms.size()-1;
          uiState = UI_ALARM_LIST;
          cursorIndex = 0;
        }
      } else {
        // NO -> go back to list
        uiState = UI_ALARM_LIST;
        cursorIndex = 0;
      }

      screenDirty = true;
    }
  }

  if (uiState == UI_WEATHER_MENU) {
    Alarm &a = tempAlarm;

    int maxIndex = 2 + a.positive.size() + a.negative.size(); // back is last

    if (steps != 0) {
        cursorIndex = constrain(cursorIndex + steps, 0, maxIndex);
        screenDirty = true;
    }

    if (clicked) {
        if (cursorIndex == 0) {
            // Add positive
            uiState = UI_WEATHER_PICK;
      weatherPickListType = 1; // positive
            cursorIndex = 0;
          clicked = false; // consume click so picker doesn't immediately confirm
        }
        else if (cursorIndex == 1) {
            // Add negative
            uiState = UI_WEATHER_PICK;
      weatherPickListType = 2; // negative
            cursorIndex = 0;
          clicked = false; // consume click so picker doesn't immediately confirm
        }
        else if (cursorIndex == maxIndex) {
            // Back
            uiState = UI_ALARM_EDIT;
            cursorIndex = 2;
        }
        else {
            // Selecting existing entry -> delete confirm
            int posCount = a.positive.size();
            if (cursorIndex - 2 < posCount) {
                // Positive entry selected
                subIndex = cursorIndex - 2; // index in positive
                editIndex = 1; // mark positive
            } else {
                // Negative entry selected
                subIndex = cursorIndex - 2 - posCount; // index in negative
                editIndex = 2; // mark negative
            }

            uiState = UI_WEATHER_DELETE_CONFIRM;
            cursorIndex = 0;
        }

        screenDirty = true;
    }
}

if (uiState == UI_WEATHER_PICK) {
  // Carousel behavior: rotate through weatherList with the encoder; click to confirm choice
  if (steps != 0) {
    // wrap around
    cursorIndex = (cursorIndex + steps) % weatherListCount;
    if (cursorIndex < 0) cursorIndex += weatherListCount;
    screenDirty = true;
  }

  if (clicked) {
    // Enter confirm screen. Save the chosen index so confirm screen can reference it.
    weatherPickChosenIndex = cursorIndex;
    cursorIndex = 0; // cursor in confirm = 0 => Save
    uiState = UI_WEATHER_PICK_CONFIRM;
    clicked = false;
    screenDirty = true;
  }
}

if (uiState == UI_WEATHER_PICK_CONFIRM) {
  if (steps != 0) {
    cursorIndex = constrain(cursorIndex + steps, 0, 1);
    screenDirty = true;
  }

  if (clicked) {
    if (cursorIndex == 0) {
      // Save chosen weather icon into the correct list
      int chosen = weatherPickChosenIndex;
      if (chosen < 0 || chosen >= weatherListCount) chosen = 0;
      WeatherIcon icon = weatherList[chosen];
      if (weatherPickListType == 1)
        tempAlarm.positive.push_back(icon);
      else
        tempAlarm.negative.push_back(icon);
    }

    // Return to weather menu after saving or cancelling
    uiState = UI_WEATHER_MENU;
    cursorIndex = 0;
    screenDirty = true;
    clicked = false;
  }
}

if (uiState == UI_WEATHER_DELETE_CONFIRM) {

    if (steps != 0) {
        cursorIndex = constrain(cursorIndex + steps, 0, 1);
        screenDirty = true;
    }

    if (clicked) {
        if (cursorIndex == 0) {
            // YES — delete
            if (editIndex == 1)
                tempAlarm.positive.erase(tempAlarm.positive.begin() + subIndex);
            else
                tempAlarm.negative.erase(tempAlarm.negative.begin() + subIndex);
        }

        uiState = UI_WEATHER_MENU;
        cursorIndex = 0;
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
      case UI_WEATHER_MENU:
        drawWeatherMenu();
        break;
      case UI_WEATHER_PICK:
        drawWeatherPick();
        break;
      case UI_WEATHER_PICK_CONFIRM:
        drawWeatherPickConfirm();
        break;
      case UI_WEATHER_DELETE_CONFIRM:
        drawWeatherDeleteConfirm();
        break;
      case UI_ALARM_DELETE_CONFIRM:
        drawAlarmDeleteConfirm();
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

  // check if any alarm should be activated (including buddy logic)
  for(int i = 0; i < alarms.size(); i++){
    Alarm &alarm = alarms[i];

    // ORIGINAL alarm: only rings when its conditions/time match
    if(hours == alarm.hour && minutes == alarm.minute && alarm.rang == false){
      bool tempOk = false;
      switch(alarm.tempCond){
        case COND_OFF:
          tempOk = true;
          break;
        case COND_LT:
          if(cachedTemp < alarm.tempValue) tempOk = true;
          break;
        case COND_GT:
          if(cachedTemp > alarm.tempValue) tempOk = true;
          break;
      }

      if(tempOk && checkWeather(alarm, mapWeatherCodeToIcon(cachedWeatherCode))){
        // original conditions satisfied -> ring original
        ring_alarm();
        alarm.rang = true;
        Serial.printf("original alarm time is %d:%d, current time is %d:%d, alarming\n", alarm.hour, alarm.minute, hours, minutes);
        if(digitalRead(TOUCH_PIN) == HIGH){
          alarm.active = false;
          stop_ring();
        }
      }
    }

    // BUDDY alarm: rings at buddy time only if buddy enabled and original has NOT rung
    if(alarm.buddyEnabled && hours == alarm.buddyHour && minutes == alarm.buddyMinute && alarm.buddyRang == false){
      if(alarm.rang == false){
        // original didn't ring earlier -> buddy must ring regardless of weather/WiFi
        ring_alarm();
        alarm.buddyRang = true;
        Serial.printf("buddy alarm time is %d:%d, current time is %d:%d, alarming\n", alarm.buddyHour, alarm.buddyMinute, hours, minutes);
        if(digitalRead(TOUCH_PIN) == HIGH){
          stop_ring();
        }
      }
    }
  }


  if (hours == 0 && minutes == 0){ //reset all alarms at midnight
    for(int i = 0; i < alarms.size(); i++){
      alarms[i].rang = false;
      alarms[i].buddyRang = false;
    }
  }

  // update touch previous state for edge detection
  prevTouched = touched;

}

