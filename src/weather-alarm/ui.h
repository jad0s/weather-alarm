#include "alarm.h"

enum UIState{
  UI_HOME,
  UI_ALARM_LIST,
  UI_ALARM_EDIT,
  UI_EDIT_FIELD
};

extern UIState uiState;
extern bool screenDirty;
extern int cursorIndex;
extern int selectedAlarmIndex;
extern std::vector<Alarm> alarms;
extern int editIndex;

void updateUi();
void updateDisplayTime();
void drawAlarmUI();
void drawAlarmList();
void drawAlarmEdit();
void editAlarmField(Alarm &a, int index, int steps);