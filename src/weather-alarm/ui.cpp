#include "ui.h"
#include "encoder.h"
#include "alarm.h"
#include "weather.h"
#include "display.h"
#include "systemtime.h"

UIState uiState = UI_HOME;
bool screenDirty = false;

int cursorIndex = 0;
int selectedAlarmIndex = 0;
int editIndex = -1; // -1 = no field being edited
int subIndex = 0; //index of the selected sub-setting


/*void drawAlarmUI(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 10);

  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", tmp_alarmHour, tmp_alarmMinute);
  display.println(buf);

  display.setCursor(0, 40);
  switch(tmp_tempCondition){
    case 0:
      display.print("OFF");
      break;
    case 1:
      display.print("<");
      break;
    case 2:
      display.print(">");
      break;
  }
  if(tmp_tempCondition != 0){
    display.setCursor(60, 40);
    display.print(tmp_tempValue);
    display.print("C");
  }

  display.display();
}*/

void drawAlarmList() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    // "+ Add" button
    if (cursorIndex == 0) display.print("> ");
    else                   display.print("  ");
    display.println("+ Add alarm");

    for (int i = 0; i < alarms.size(); i++) {
        if (cursorIndex == i+1) display.print("> ");
        else                    display.print("  ");

        char buf[8];
        snprintf(buf, sizeof(buf), "%02d:%02d", alarms[i].hour, alarms[i].minute);
        display.print(i+1);
        display.print(") ");
        display.print(buf);
        display.print(alarms[i].enabled ? " *" : "");
        display.println();
    }

    display.display();
}


void drawAlarmEdit() {
    // guard: if no alarms, go back to list
    if (alarms.empty()) {
        uiState = UI_ALARM_LIST;
        cursorIndex = 0;
        screenDirty = true;
        return;
    }

    // clamp selectedAlarmIndex
    if (selectedAlarmIndex < 0) selectedAlarmIndex = 0;
    if (selectedAlarmIndex >= (int)alarms.size()) selectedAlarmIndex = alarms.size() - 1;

    Alarm &a = tempAlarm;

    display.clearDisplay();
    display.setTextSize(1);

    // keep cursor within [0..4]
    cursorIndex = constrain(cursorIndex, 0, 4);

    int y = 0;

    auto line = [&](const char *label, int index) {
        display.setCursor(0, y);
        if (cursorIndex == index) display.print("> ");
        else display.print("  ");
        display.println(label);
        y += 12;
    };

    char buf[24];

    // Time line
    snprintf(buf, sizeof(buf), "Time: %02d:%02d", a.hour, a.minute);
    line(buf, 0);

    // Temp cond + value line
    const char *condSym = " ";
    if (a.tempCond == COND_LT) condSym = "<";
    else if (a.tempCond == COND_GT) condSym = ">";
    // show like: "Temp: <  5C"
    snprintf(buf, sizeof(buf), "Temp: %s %dC", condSym, a.tempValue);
    line(buf, 1);

    // Weather count line
    int totalWeather = a.positive.size() + a.negative.size();
    snprintf(buf, sizeof(buf), "Weather: %d", totalWeather);
    line(buf, 2);

    line("Delete", 3);
    line("Back", 4);

    display.display();
}


void updateDisplayTime(){
  if(!screenOn){
    return;
  }
    display.clearDisplay();           // clear previous time
    display.setTextSize(3);
    display.setCursor(0, 0);

    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hours, minutes);
    display.println(buf);
    display.setTextSize(2);
    display.setCursor(0, 30);         // adjust vertical position

    if(weatherValid){
        char weatherBuf[32];
        snprintf(weatherBuf, sizeof(weatherBuf),
                 "%.1fC \n%.1fmm/h",
                 cachedTemp, cachedRain);
        display.println(weatherBuf);
    } else {
        display.println("Weather N/A");
    }
    display.display();
}

void editAlarmField(int index, int subIndex, int steps) {
    Alarm &a = tempAlarm;

    switch (index) {

        case 0: // Time row
            if (subIndex == 0) {
                // hour
                a.hour = (a.hour + steps + 24) % 24;
            } else if (subIndex == 1) {
                // minute
                a.minute = (a.minute + steps + 60) % 60;
            }
            break;

        case 1: // Temperature row
            if (subIndex == 0) {
                // temp condition (OFF, <, >)
                a.tempCond = (TempCondition) constrain((int)a.tempCond + steps, 0, 2);
            } else if (subIndex == 1) {
                // temp value
                if (a.tempCond != COND_OFF)
                    a.tempValue = constrain(a.tempValue + steps, -40, 60);
            }
            break;
        // 3 = Delete  (handled outside)
        // 4 = Back    (handled outside)

    }
}

void drawWeatherMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);

    Alarm &a = tempAlarm;

    int row = 0;
    auto printRow = [&](const char* label, int idx) {
        display.setCursor(0, row);
        if (cursorIndex == idx) display.print("> ");
        else display.print("  ");
        display.println(label);
        row += 12;
    };

    printRow("+ Add POSITIVE", 0);
    printRow("+ Add NEGATIVE", 1);

    int index = 2;

    // Print positive list
    for (int i = 0; i < a.positive.size(); i++) {
        char lineBuf[32];
        snprintf(lineBuf, sizeof(lineBuf), "POS: %d", a.positive[i]);
        printRow(lineBuf, index++);
    }

    // Print negative list
    for (int i = 0; i < a.negative.size(); i++) {
        char lineBuf[32];
        snprintf(lineBuf, sizeof(lineBuf), "NEG: %d", a.negative[i]);
        printRow(lineBuf, index++);
    }

    // Back
    printRow("Back", index);

    display.display();
}

const WeatherCode weatherList[] = {
    WC_CLEAR, WC_MAINLY_CLEAR, WC_PARTLY_CLOUDY, WC_OVERCAST,
    WC_FOG, WC_RIME_FOG, WC_DRIZZLE_LIGHT, WC_DRIZZLE_MOD,
    WC_DRIZZLE_DENSE, WC_RAIN_SLIGHT, WC_RAIN_MOD, WC_RAIN_HEAVY,
    WC_SHOWER_SLIGHT, WC_SHOWER_MOD, WC_SHOWER_VIOLENT,
    WC_STORM, WC_STORM_HAIL_SLIGHT, WC_STORM_HAIL_HEAVY
};

const int weatherListCount = sizeof(weatherList)/sizeof(weatherList[0]);

void drawWeatherPick() {
    display.clearDisplay();
    display.setTextSize(1);

    int row = 0;
    for (int i = 0; i < weatherListCount; i++) {
        display.setCursor(0, row);
        if (cursorIndex == i) display.print("> ");
        else display.print("  ");
        display.print("Code ");
        display.println((int)weatherList[i]);
        row += 12;
    }

    // Back option
    display.setCursor(0,row);
    if (cursorIndex == weatherListCount) display.print("> ");
    else display.print("  ");
    display.println("Back");

    display.display();
}

void drawWeatherDeleteConfirm() {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0,0);
    display.println("Delete?");
    
    display.setCursor(0,12);
    if (cursorIndex == 0) display.print("> ");
    else display.print("  ");
    display.println("Yes");

    display.setCursor(0,24);
    if (cursorIndex == 1) display.print("> ");
    else display.print("  ");
    display.println("No");

    display.display();
}






