#ifndef APPTYPES_H
#define APPTYPES_H

enum AppState {
  STATE_MENU,
  STATE_POMODORO,
  STATE_CALENDAR,
  STATE_FLASHCARDS,
  STATE_CALCULATOR,
  STATE_ALERTS,
  STATE_CHAT,
  STATE_SETTINGS,
  STATE_POWER_OFF
};

enum SettingsMenu {
  SETTINGS_MAIN,
  SETTINGS_WIFI,
  SETTINGS_OTA,
  SETTINGS_TUNE,
  SETTINGS_SYSINFO
};

struct MenuItem {
  const char* label;
  const unsigned char* icon;
  AppState targetState;
};

#endif // APPTYPES_H
