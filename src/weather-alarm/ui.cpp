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

std::vector<Alarm> alarms;

void drawAlarmUI(){
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
}

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
    Alarm &a = alarms[selectedAlarmIndex];

    display.clearDisplay();
    display.setTextSize(1);

    int y = 0;

    auto line = [&](const char *label, int index) {
        display.setCursor(0, y);
        if (cursorIndex == index) display.print("> ");
        else display.print("  ");
        display.println(label);
        y += 12;
    };

    char buf[16];

    snprintf(buf, sizeof(buf), "Time: %02d:%02d", a.hour, a.minute);
    line(buf, 0);

    snprintf(buf, sizeof(buf), "Temp: %c %dC",
             a.tempCond == COND_LT ? '<' : (a.tempCond == COND_GT ? '>' : ' '),
             a.tempValue);
    line(buf, 1);

    snprintf(buf, sizeof(buf), "Weather: %d conds", a.weather.size());
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

void editAlarmField(Alarm &a, int index, int steps) {
    switch(index) {
        case 0: // time
            a.hour = (a.hour + steps + 24) % 24;
            break;
        case 1: // temp
            a.tempValue = constrain(a.tempValue + steps, -30, 50);
            break;
        case 2: // weather count or navigating to weather menu
            // optional later
            break;
        case 3: // delete
            // nothing to edit; click will delete
            break;
        case 4: // back
            // nothing to edit
            break;
    }
}

