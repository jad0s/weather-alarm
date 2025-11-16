enum UIState{
  UI_HOME,
  UI_SET_ALARM
};

extern UIState uiState;
extern bool screenDirty;

void updateUi();
void updateDisplayTime();
void drawAlarmUI();