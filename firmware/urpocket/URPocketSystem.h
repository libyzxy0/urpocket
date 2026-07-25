#ifndef URPOCKETSYSTEM_H
#define URPOCKETSYSTEM_H

class URPocketSystem {
private:
  Adafruit_SH1106G display;
  AudioSystem audio;
  AppNetworkManager network;
  InputController input;

  PomodoroApp pomodoroApp;
  CalendarApp calendarApp;
  FlashcardsApp flashcardsApp;
  CalculatorApp calculatorApp;
  AlertsApp alertsApp;
  ChatApp chatApp;
  SettingsApp settingsApp;
  PowerApp powerApp;

  AppState currentState = STATE_MENU;
  int currentSelection = 0;
  int scrollOffset = 0;
  bool isPoweredOn = false;

  static const int NUM_ITEMS = 8;
  MenuItem menuItems[NUM_ITEMS] = {
    {"Pomodoro",      icon_pomodoro,   STATE_POMODORO},
    {"Calendar",      icon_calendar,   STATE_CALENDAR},
    {"Flashcards",    icon_flashcards, STATE_FLASHCARDS},
    {"Calculator",    icon_calculator, STATE_CALCULATOR},
    {"Alerts",        icon_bell,       STATE_ALERTS},
    {"OrensAI",       icon_message,    STATE_CHAT},
    {"Settings",      icon_gear,       STATE_SETTINGS},
    {"Power Options", icon_power,      STATE_POWER_OFF}
  };

  void drawSplashScreen() {
    const unsigned long duration = 3000;
    const int maxProgressWidth = 65;
    unsigned long startTime = millis();
    audio.playBootSound();

    while (millis() - startTime < duration) {
      display.clearDisplay();
      display.drawBitmap(-7, 2, image_logo, 64, 64, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
      display.setTextWrap(false);
      display.setCursor(65, 18);
      display.print("URPocket");
      display.setCursor(58, 36);
      display.print("1.0.0 LTS");

      display.drawBitmap(56, 13, image_space, 65, 18, SH110X_WHITE);
      display.drawRect(54, 48, 69, 8, SH110X_WHITE);

      unsigned long elapsed = millis() - startTime;
      int fillWidth = map(elapsed, 0, duration, 0, maxProgressWidth);
      if (fillWidth > maxProgressWidth) fillWidth = maxProgressWidth;
      display.fillRect(56, 50, fillWidth, 4, SH110X_WHITE);

      display.display();
      delay(30);
    }

    display.clearDisplay();
    display.drawBitmap(12, 4, image_certification, 103, 56, SH110X_WHITE);
    display.display();
    delay(1500);
  }

  void drawMenu() {
    display.clearDisplay();
    int itemHeight = 20;

    for (int i = 0; i < 3; i++) {
      int itemIndex = scrollOffset + i;
      if (itemIndex >= NUM_ITEMS) break;

      int yPos = i * itemHeight + 2;

      if (itemIndex == currentSelection) {
        display.fillRect(0, yPos - 1, SCREEN_WIDTH, itemHeight, SH110X_WHITE);
        display.setTextColor(SH110X_BLACK);
      } else {
        display.setTextColor(SH110X_WHITE);
      }

      display.drawBitmap(4, yPos, menuItems[itemIndex].icon, 16, 16, (itemIndex == currentSelection) ? SH110X_BLACK : SH110X_WHITE);
      display.setTextSize(1);
      display.setCursor(26, yPos + 4);
      display.print(menuItems[itemIndex].label);
    }

    if (NUM_ITEMS > 3) {
      int barHeight = (64 / NUM_ITEMS) * 3;
      int barPos = (64 - barHeight) * currentSelection / (NUM_ITEMS - 1);
      display.fillRect(125, barPos, 3, barHeight, SH110X_WHITE);
    }

    display.display();
  }

  void handleMenuState(bool up, bool down, bool select) {
    if (down) {
      currentSelection++;
      if (currentSelection >= NUM_ITEMS) {
        currentSelection = 0;
        scrollOffset = 0;
      }
      if (currentSelection >= scrollOffset + 3) {
        scrollOffset = currentSelection - 2;
      }
      drawMenu();
    }
    if (up) {
      currentSelection--;
      if (currentSelection < 0) {
        currentSelection = NUM_ITEMS - 1;
        scrollOffset = NUM_ITEMS - 3;
      }
      if (currentSelection < scrollOffset) {
        scrollOffset = currentSelection;
      }
      drawMenu();
    }
    if (select) {
      audio.playClickSound();
      currentState = menuItems[currentSelection].targetState;
      if (currentState == STATE_SETTINGS) {
        settingsApp.resetSubState();
      }
    }
  }

public:
  URPocketSystem()
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      audio(BUZZER_PIN),
      input(7, 6, 5) {}

  void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Wire.begin(I2C_SDA, I2C_SCL);

    if (!display.begin(SCREEN_ADDRESS, true)) {
      for (;;);
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    if (!isPoweredOn) {
      drawSplashScreen();
      isPoweredOn = true;
    }

    digitalWrite(LED_PIN, HIGH);
    drawMenu();

    input.begin();
  }

  void loop() {
    bool upPressed = false;
    bool downPressed = false;
    bool selectPressed = false;
    bool selectLongPressed = false;
    bool selectDoubleClicked = false;

    input.update(upPressed, downPressed, selectPressed, selectLongPressed, selectDoubleClicked);

    if (currentState != STATE_MENU && selectLongPressed) {
      currentState = STATE_MENU;
      settingsApp.resetSubState();
      drawMenu();
      ArduinoOTA.end();
      return;
    }

    switch (currentState) {
      case STATE_MENU:
        handleMenuState(upPressed, downPressed, selectPressed);
        break;
      case STATE_POMODORO:
        pomodoroApp.run(display, audio, upPressed, downPressed, selectPressed);
        break;
      case STATE_CALENDAR:
        calendarApp.run(display, network, audio, upPressed, downPressed, selectPressed, selectDoubleClicked);
        break;
      case STATE_FLASHCARDS:
        flashcardsApp.run(display, audio, upPressed, downPressed, selectPressed);
        break;
      case STATE_CALCULATOR:
        calculatorApp.run(display, audio, upPressed, downPressed, selectPressed);
        break;
      case STATE_CHAT:
        chatApp.run(display, network, audio, upPressed, downPressed, selectPressed, selectDoubleClicked);
        break;
      case STATE_SETTINGS:
        settingsApp.run(display, network, audio, currentState, upPressed, downPressed, selectPressed);
        break;
      case STATE_ALERTS:
        alertsApp.run(display, audio, upPressed, downPressed, selectPressed);
        break;
      case STATE_POWER_OFF:
        powerApp.run(display, audio, upPressed, downPressed, selectPressed);
        break;
    }
  }
};

#endif // URPOCKETSYSTEM_H
