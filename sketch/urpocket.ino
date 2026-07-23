#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h> 
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

#define I2C_SDA 3
#define I2C_SCL 4

#define BUZZER_PIN 1
#define LED_PIN 8

const char* ssid = "OrangeCat";
const char* password = "myorange32";

bool wifiEnabled = false;
bool otaEnabled = false;
bool otaConnecting = false;
bool otaConnected = false;
bool otaUpdating = false;
int otaProgressPercent = 0;
String otaStatusText = "SEL to Start OTA";

enum SettingsMenu {
  SETTINGS_MAIN,
  SETTINGS_WIFI,
  SETTINGS_OTA,
  SETTINGS_TUNE
};
SettingsMenu currentSettingsSubState = SETTINGS_MAIN;
int settingsSelection = 0;

int powerMenuSelection = 0;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int BUTTON_UP = 5;
const int BUTTON_DOWN = 6;
const int BUTTON_SELECT = 7;

static const unsigned char PROGMEM icon_calendar[] = {0x09,0x20,0x76,0xdc,0xff,0xfe,0xff,0xfe,0x80,0x02,0x86,0xda,0x86,0xda,0x80,0x02,0xb6,0xda,0xb6,0xda,0x80,0x02,0xb6,0xc2,0xb6,0xc2,0x80,0x02,0x7f,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x04,0x10,0x00,0x00,0x00,0x00,0x00,0x80};
static const unsigned char PROGMEM icon_power[] = {0x01,0x00,0x01,0x00,0x19,0x30,0x25,0x48,0x49,0x24,0x51,0x14,0xa1,0x0a,0xa0,0x0a,0xa0,0x0a,0xa0,0x0a,0xa0,0x0a,0x50,0x14,0x48,0x24,0x27,0xc8,0x18,0x30,0x07,0xc0};
static const unsigned char PROGMEM icon_gear[] = {0x03,0xc0,0x12,0x48,0x2c,0x34,0x40,0x02,0x23,0xc4,0x24,0x24,0xc8,0x13,0x88,0x11,0x88,0x11,0xc8,0x13,0x24,0x24,0x23,0xc4,0x40,0x02,0x2c,0x34,0x12,0x48,0x03,0xc0};
static const unsigned char PROGMEM icon_message[] = {0x7f,0xfe,0x80,0x02,0x80,0x01,0x96,0xd1,0x9f,0xf9,0x80,0x01,0x80,0x01,0x95,0xb1,0x9f,0xf9,0x80,0x01,0x80,0x01,0x63,0xfe,0x14,0x00,0x08,0x00,0x10,0x00,0x00,0x00};
static const unsigned char PROGMEM icon_flashcards[] = {0x7f,0xfc,0xc0,0x06,0xff,0xf2,0x80,0x0a,0x80,0x0a,0x9f,0xea,0x90,0x2a,0x9f,0xea,0x80,0x0a,0x80,0x0a,0x8f,0x8a,0x80,0x0a,0x80,0x0a,0x8f,0x8a,0x80,0x0c,0x7f,0xf8};
static const unsigned char PROGMEM icon_bell[] = {0x11,0x88,0x63,0xc6,0x44,0x22,0x8c,0x11,0x88,0x11,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x20,0x04,0x40,0x02,0xff,0xff,0x06,0x60,0x03,0xc0,0x00,0x00};
static const unsigned char PROGMEM icon_pomodoro[] = {0xff,0xe0,0x40,0x40,0x40,0x40,0x51,0x40,0x5f,0x40,0x2e,0x80,0x15,0x00,0x0a,0x00,0x0a,0x00,0x11,0x00,0x24,0x80,0x44,0x40,0x4e,0x40,0x5f,0x40,0x7f,0xc0,0xff,0xe0};
static const unsigned char PROGMEM icon_calculator[] = {0x7f,0xe0,0x80,0x10,0xbf,0xd0,0xa0,0x50,0xbf,0xd0,0x80,0x10,0xb6,0xd0,0xb6,0xd0,0x80,0x10,0xb6,0xd0,0xb6,0xd0,0x80,0x10,0xb6,0xd0,0xb6,0xd0,0x80,0x10,0x7f,0xe0};

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

AppState currentState = STATE_POWER_OFF;

struct MenuItem {
  const char* label;
  const unsigned char* icon;
  AppState targetState;
};

const int NUM_ITEMS = 8;
MenuItem menuItems[NUM_ITEMS] = {
  {"Pomodoro",   icon_pomodoro,   STATE_POMODORO},
  {"Calendar",   icon_calendar,   STATE_CALENDAR},
  {"Flashcards", icon_flashcards, STATE_FLASHCARDS},
  {"Calculator", icon_calculator, STATE_CALCULATOR},
  {"Alerts",     icon_bell,       STATE_ALERTS},
  {"ChatGpt",   icon_message,     STATE_CHAT},
  {"Settings",   icon_gear,       STATE_SETTINGS},
  {"Power Options", icon_power,   STATE_POWER_OFF}
};

int currentSelection = 0;
int scrollOffset = 0;

bool lastUpState = HIGH;
bool lastDownState = HIGH;
bool lastSelectState = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;

unsigned long selectPressedTime = 0;
bool isLongPressHandled = false;
const unsigned long longPressDelay = 600;

unsigned long lastSelectReleaseTime = 0;
bool selectClickPending = false;
const unsigned long doubleClickDelay = 250;
bool selectDoubleClicked = false;

bool isPoweredOn = false;

void handleMenuState(bool up, bool down, bool select);
void drawMenu();
void drawOTAUIPage();
void stopOTA();
void runPomodoro(bool up, bool down, bool select);
void runCalendar(bool up, bool down, bool select);
void runFlashcards(bool up, bool down, bool select);
void runChat(bool up, bool down, bool select);
void runSettings(bool up, bool down, bool select);
void runAlerts(bool up, bool down, bool select);
void runPowerOff(bool up, bool down, bool select);
void turnOffDevice();
void turnOnDevice();

String fetchHttpsApi(const char* url) {
  if (WiFi.status() != WL_CONNECTED) {
    return "Error: No WiFi";
  }

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate verification for simple requests

  HTTPClient http;
  if (http.begin(client, url)) {
    int httpCode = http.GET();
    String payload = "";
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
      } else {
        payload = "HTTP Error: " + String(httpCode);
      }
    } else {
      payload = "Conn Failed: " + String(http.errorToString(httpCode));
    }
    http.end();
    return payload;
  }
  return "Error: Invalid URL";
}

void playClickSound() {
  tone(BUZZER_PIN, 1800, 15);
}

void playBootSound() {
  tone(BUZZER_PIN, 1047, 100);
  delay(120);
  tone(BUZZER_PIN, 1318, 100);
  delay(120);
  tone(BUZZER_PIN, 1568, 150);
  delay(150);
  noTone(BUZZER_PIN);
}

void playHappyBirthday() {
  int notes[] = {
    262, 262, 294, 262, 349, 330,
    262, 262, 294, 262, 392, 349,
    262, 262, 523, 440, 349, 330, 294,
    466, 466, 440, 349, 392, 349
  };

  int durations[] = {
    250, 250, 500, 500, 500, 1000,
    250, 250, 500, 500, 500, 1000,
    250, 250, 500, 500, 500, 500, 1000,
    250, 250, 500, 500, 500, 1000
  };

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(15, 25);
  display.setTextSize(1);
  display.print("Playing Tune...");
  display.display();

  for (int i = 0; i < 25; i++) {
    digitalWrite(LED_PIN, (i % 2 == 0) ? HIGH : LOW);
    tone(BUZZER_PIN, notes[i], durations[i] * 0.85);
    delay(durations[i]);
    noTone(BUZZER_PIN);
  }
  digitalWrite(LED_PIN, HIGH);
}

void toggleWiFi() {
  wifiEnabled = !wifiEnabled;
  if (wifiEnabled) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  } else {
    stopOTA();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
}

void drawOTAUIPage() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("OTA STATUS");
  display.print(otaEnabled ? "    ENABLED" : "   DISABLED");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);

  display.setCursor(0, 14);
  display.print(otaStatusText);

  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(0, 26);
    display.print(WiFi.localIP());
  }

  if (otaUpdating) {
    display.drawRect(0, 40, 128, 12, SH110X_WHITE);
    int fillWidth = map(otaProgressPercent, 0, 100, 0, 124);
    display.fillRect(2, 42, fillWidth, 8, SH110X_WHITE);
    
    display.setCursor(0, 54);
    display.print("Progress");

    if(otaProgressPercent >= 10) {
      display.setCursor(110, 54);
    } else if (otaProgressPercent >= 100) {
      display.setCursor(105, 54);
    } else {
      display.setCursor(115, 54);
    }
    display.print(otaProgressPercent);
    display.print("%");
  } else {
    display.setCursor(0, 50);
    if (!wifiEnabled) {
      display.print("Turn WiFi ON First!");
    } else if (!otaEnabled) {
      display.print("Click SEL to Enable");
    } else {
      display.print("Waiting Upload...");
    }
  }
  display.display();
}

void setupOTAHandlers() {
  ArduinoOTA.onStart([]() {
    otaUpdating = true;
    otaStatusText = "Updating Firmware...";
    drawOTAUIPage();
  });

  ArduinoOTA.setPassword("libyzxy0");

  ArduinoOTA.onEnd([]() {
    otaUpdating = false;
    otaStatusText = "Update Complete!";
    drawOTAUIPage();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    otaProgressPercent = (progress / (total / 100));
    drawOTAUIPage();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaUpdating = false;
    if (error == OTA_AUTH_ERROR) otaStatusText = "Auth Failed";
    else if (error == OTA_BEGIN_ERROR) otaStatusText = "Begin Failed";
    else if (error == OTA_CONNECT_ERROR) otaStatusText = "Connect Failed";
    else if (error == OTA_RECEIVE_ERROR) otaStatusText = "Receive Failed";
    else if (error == OTA_END_ERROR) otaStatusText = "End Failed";
    drawOTAUIPage();
  });
}

void startOTA() {
  if (wifiEnabled && !otaEnabled) {
    setupOTAHandlers();
    ArduinoOTA.begin();
    otaEnabled = true;
    otaStatusText = "Listening for OTA...";
  }
}

void stopOTA() {
  otaEnabled = false;
  otaConnecting = false;
  otaConnected = false;
  otaUpdating = false;
  otaStatusText = "OTA Disabled";
}

void setup() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_OFF);
  Wire.begin(I2C_SDA, I2C_SCL);
  display.begin(SCREEN_ADDRESS, true);

  turnOnDevice();
}

void playPowerOffSound() {
  tone(BUZZER_PIN, 1568, 100);
  delay(120);
  tone(BUZZER_PIN, 1318, 100);
  delay(120);
  tone(BUZZER_PIN, 1047, 150);
  delay(150);
  noTone(BUZZER_PIN);
}

void turnOffDevice() {
  playPowerOffSound();
  isPoweredOn = false;
  wifiEnabled = false;
  stopOTA();
  WiFi.mode(WIFI_OFF);
  digitalWrite(LED_PIN, LOW);
  display.clearDisplay();
  display.display();
  display.oled_command(SH110X_DISPLAYOFF);
  currentState = STATE_POWER_OFF;
}

void turnOnDevice() {
  display.oled_command(SH110X_DISPLAYON);
  digitalWrite(LED_PIN, HIGH);
  isPoweredOn = true;
  currentState = STATE_MENU;
  currentSelection = 0;
  scrollOffset = 0;
  powerMenuSelection = 0;
  currentSettingsSubState = SETTINGS_MAIN;
  settingsSelection = 0;
  playBootSound();
  drawMenu();
}

void loop() {
  if (currentState == STATE_SETTINGS && currentSettingsSubState == SETTINGS_OTA && wifiEnabled && otaEnabled) {
    ArduinoOTA.handle();
  }

  bool upPressed = false;
  bool downPressed = false;
  bool selectPressed = false;
  bool selectLongPressed = false;
  selectDoubleClicked = false;

  if (selectClickPending && (millis() - lastSelectReleaseTime > doubleClickDelay)) {
    selectPressed = true;
    selectClickPending = false;
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    bool upState = digitalRead(BUTTON_UP);
    bool downState = digitalRead(BUTTON_DOWN);
    bool selectState = digitalRead(BUTTON_SELECT);

    if (lastUpState == HIGH && upState == LOW) {
      upPressed = true;
      lastDebounceTime = millis();
    }
    if (lastDownState == HIGH && downState == LOW) {
      downPressed = true;
      lastDebounceTime = millis();
    }
    
    if (lastSelectState == HIGH && selectState == LOW) {
      selectPressedTime = millis();
      isLongPressHandled = false;
      if (selectClickPending && (millis() - lastSelectReleaseTime <= doubleClickDelay)) {
        selectDoubleClicked = true;
        selectClickPending = false;
        isLongPressHandled = true;
      }
    }
    
    if (selectState == LOW && !isLongPressHandled) {
      if (millis() - selectPressedTime > longPressDelay) {
        selectLongPressed = true;
        isLongPressHandled = true;
      }
    }
    
    if (lastSelectState == LOW && selectState == HIGH) {
      if (!isLongPressHandled) {
        selectClickPending = true;
        lastSelectReleaseTime = millis();
      }
    }

    lastUpState = upState;
    lastDownState = downState;
    lastSelectState = selectState;
  }

  if (!isPoweredOn) {
    if (selectPressed || selectLongPressed) {
      turnOnDevice();
    }
    return;
  }

  if (currentState != STATE_MENU && selectLongPressed) {
    stopOTA();
    currentState = STATE_MENU;
    currentSettingsSubState = SETTINGS_MAIN;
    drawMenu();
    return;
  }

  switch (currentState) {
    case STATE_MENU:
      handleMenuState(upPressed, downPressed, selectPressed);
      break;
    case STATE_POMODORO:
      runPomodoro(upPressed, downPressed, selectPressed);
      break;
    case STATE_CALENDAR:
      runCalendar(upPressed, downPressed, selectPressed);
      break;
    case STATE_FLASHCARDS:
      runFlashcards(upPressed, downPressed, selectPressed);
      break;
    case STATE_CALCULATOR:
      runCalculator(upPressed, downPressed, selectPressed);
      break;
    case STATE_CHAT:
      runChat(upPressed, downPressed, selectPressed || selectDoubleClicked);
      break;
    case STATE_SETTINGS:
      runSettings(upPressed, downPressed, selectPressed);
      break;
    case STATE_ALERTS:
      runAlerts(upPressed, downPressed, selectPressed);
      break;
    case STATE_POWER_OFF:
      runPowerOff(upPressed, downPressed, selectPressed);
      break;
  }
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
      if (scrollOffset < 0) scrollOffset = 0;
    }
    if (currentSelection < scrollOffset) {
      scrollOffset = currentSelection;
    }
    drawMenu();
  }

  if (select) {
    playClickSound();
    currentState = menuItems[currentSelection].targetState;
    if (currentState != STATE_POWER_OFF) {
      display.clearDisplay();
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);
      display.setCursor(10, 20);
      display.print("Opening...");
      display.setCursor(10, 36);
      display.setTextSize(2);
      display.print(menuItems[currentSelection].label);
      display.display();
    }
  }
}

void drawMenu() {
  display.clearDisplay();
  const int itemHeight = 20;
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

    display.drawBitmap(4, yPos, menuItems[itemIndex].icon, 16, 16, 
                       (itemIndex == currentSelection) ? SH110X_BLACK : SH110X_WHITE);
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

void runPomodoro(bool up, bool down, bool select) {
  static unsigned long lastTimerUpdate = 0;
  static int secondsLeft = 1500;
  if (millis() - lastTimerUpdate >= 1000) {
    lastTimerUpdate = millis();
    if (secondsLeft > 0) secondsLeft--;
  }

  display.clearDisplay();
  display.setCursor(10, 10);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("POMODORO TIMER");
  display.setCursor(10, 30);
  display.setTextSize(2);
  display.print(secondsLeft / 60);
  display.print(":");
  if(secondsLeft % 60 < 10) display.print("0");
  display.print(secondsLeft % 60);
  
  display.setTextSize(1);
  display.setCursor(10, 52);
  display.print("Hold SELECT to exit");
  display.display();
}

void runCalendar(bool up, bool down, bool select) {
  static bool calendarLoaded = false;
  static bool viewingDetail = false;
  static int currentEventIndex = 0;
  static int totalEvents = 0;
  struct LocalEvent {
    String summary;
    String code;
    String startTime;
    String timeRange; 
    String location;
  };
  static LocalEvent events[5];

  if (!calendarLoaded) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.print("Fetching Schedule...");
    display.display();

    String jsonResponse = fetchHttpsApi("https://urpocket.libyzxy0.me/api/calendar/today");
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonResponse);

    totalEvents = 0;
    currentEventIndex = 0;
    viewingDetail = false;

    if (!error && doc.is<JsonArray>()) {
      JsonArray array = doc.as<JsonArray>();
      for (JsonObject obj : array) {
        if (totalEvents >= 5) break;

        String summary = obj["summary"].as<String>();
        String startStr = obj["start"].as<String>();
        String endStr = obj["end"].as<String>();

        int startT = startStr.indexOf('T');
        int endT = endStr.indexOf('T');
        String startTime = (startT != -1 && startStr.length() >= startT + 6) ?
          startStr.substring(startT + 1, startT + 6) : startStr;
        String endTime = (endT != -1 && endStr.length() >= endT + 6) ?
          endStr.substring(endT + 1, endT + 6) : endStr;

        int dashIdx = summary.indexOf(" - ");
        String code = (dashIdx != -1) ? summary.substring(0, dashIdx) : summary;

        events[totalEvents].summary = summary;
        events[totalEvents].code = code;
        events[totalEvents].startTime = startTime;
        events[totalEvents].timeRange = startTime + " - " + endTime;
        events[totalEvents].location = obj["location"].as<String>();

        totalEvents++;
      }
    }

    calendarLoaded = true;
  }

  if (viewingDetail) {
    if (up || down || select) {
      playClickSound();
      viewingDetail = false;
    }
  } else {
    if (down && totalEvents > 0) {
      playClickSound();
      currentEventIndex = (currentEventIndex + 1) % totalEvents;
    }
    if (up && totalEvents > 0) {
      playClickSound();
      currentEventIndex = (currentEventIndex - 1 + totalEvents) % totalEvents;
    }
    if (select && totalEvents > 0) {
      playClickSound();
      viewingDetail = true;
    }
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setTextWrap(true);

  if (totalEvents == 0) {
    display.setCursor(10, 25);
    display.print("No Events Found!");
  } else if (viewingDetail) {
    LocalEvent &e = events[currentEventIndex];

    display.setCursor(0, 0);
    display.print(e.code);
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);

    display.setCursor(0, 14);
    display.print(e.summary);

    display.setCursor(0, 32);
    display.print("TIME: ");
    display.print(e.timeRange);

    display.setCursor(0, 42);
    display.print(e.location);
  } else {
    display.setCursor(0, 0);
    display.print("SCHEDULE");
    String countStr = String(totalEvents) + (totalEvents == 1 ? " event" : " events");
    display.setCursor(128 - (countStr.length() * 6), 0);
    display.print(countStr);
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);

    for (int i = 0; i < totalEvents; i++) {
      int y = 14 + (i * 10);
      bool selected = (i == currentEventIndex);

      if (selected) {
        display.fillRect(0, y - 1, 128, 9, SH110X_WHITE);
        display.setTextColor(SH110X_BLACK);
      } else {
        display.setTextColor(SH110X_WHITE);
      }

      display.setCursor(2, y);
      display.print(events[i].code);

      String t = events[i].startTime;
      display.setCursor(128 - (t.length() * 6) - 2, y);
      display.print(t);
    }

    display.setTextColor(SH110X_WHITE);
  }

  display.display();
}

void runFlashcards(bool up, bool down, bool select) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 20);
  display.setTextSize(1);
  display.print("Flashcards App");
  display.setCursor(10, 45);
  display.print("Hold SELECT to exit");
  display.display();
}

void runCalculator(bool up, bool down, bool select) {
  static double num1 = 0;
  static double num2 = 0;
  static double result = 0;
  static char op = ' ';
  static char currentInput[16] = "0";
  static int inputLen = 1;
  static bool hasDecimal = false;
  static bool startNewNum = true;
  static bool hasError = false;

  static int cursorIndex = 0; // Sequential index (0 to 18)

  const char keys[5][4] = {
    {'C', '<', '%', '/'},
    {'7', '8', '9', '*'},
    {'4', '5', '6', '-'},
    {'1', '2', '3', '+'},
    {'0', '.', '=', '='}
  };

  // Total valid buttons on grid (row 4, col 3 '=' is merged with col 2)
  const int totalKeys = 19;

  // Navigate sequentially across all keys using UP and DOWN
  if (down) {
    cursorIndex = (cursorIndex + 1) % totalKeys;
  }
  if (up) {
    cursorIndex = (cursorIndex - 1 + totalKeys) % totalKeys;
  }

  // Convert 1D cursor index to 2D grid row/col
  int gridRow = cursorIndex / 4;
  int gridCol = cursorIndex % 4;

  // Handle merged '=' button at the bottom right
  if (gridRow == 4 && gridCol == 3) {
    cursorIndex = 18; // Wrap to the merged '=' button
    gridRow = 4;
    gridCol = 2;
  }

  if (select) {
    playClickSound();
    char k = keys[gridRow][gridCol];

    if (hasError && k != 'C') {
      k = 'C';
    }

    if (k >= '0' && k <= '9') {
      if (startNewNum) {
        currentInput[0] = k;
        currentInput[1] = '\0';
        inputLen = 1;
        hasDecimal = false;
        startNewNum = false;
      } else if (inputLen < 12) {
        if (inputLen == 1 && currentInput[0] == '0') {
          currentInput[0] = k;
        } else {
          currentInput[inputLen] = k;
          inputLen++;
          currentInput[inputLen] = '\0';
        }
      }
    } else if (k == '.') {
      if (startNewNum) {
        currentInput[0] = '0';
        currentInput[1] = '.';
        currentInput[2] = '\0';
        inputLen = 2;
        hasDecimal = true;
        startNewNum = false;
      } else if (!hasDecimal && inputLen < 12) {
        currentInput[inputLen] = '.';
        inputLen++;
        currentInput[inputLen] = '\0';
        hasDecimal = true;
      }
    } else if (k == '<') {
      if (!startNewNum && inputLen > 0) {
        if (currentInput[inputLen - 1] == '.') {
          hasDecimal = false;
        }
        inputLen--;
        currentInput[inputLen] = '\0';
        if (inputLen == 0) {
          currentInput[0] = '0';
          currentInput[1] = '\0';
          inputLen = 1;
          startNewNum = true;
        }
      }
    } else if (k == 'C') {
      num1 = 0;
      num2 = 0;
      result = 0;
      op = ' ';
      currentInput[0] = '0';
      currentInput[1] = '\0';
      inputLen = 1;
      hasDecimal = false;
      startNewNum = true;
      hasError = false;
    } else if (k == '+' || k == '-' || k == '*' || k == '/' || k == '%') {
      if (!startNewNum && op != ' ') {
        num2 = atof(currentInput);
        if (op == '/' && num2 == 0) {
          hasError = true;
        } else {
          if (op == '+') result = num1 + num2;
          else if (op == '-') result = num1 - num2;
          else if (op == '*') result = num1 * num2;
          else if (op == '/') result = num1 / num2;
          else if (op == '%') result = fmod(num1, num2);

          num1 = result;
          dtostrf(result, 1, 4, currentInput);
          int l = strlen(currentInput);
          while (l > 1 && currentInput[l - 1] == '0') {
            currentInput[l - 1] = '\0';
            l--;
          }
          if (l > 1 && currentInput[l - 1] == '.') {
            currentInput[l - 1] = '\0';
            l--;
          }
          inputLen = strlen(currentInput);
        }
      } else {
        num1 = atof(currentInput);
      }
      op = k;
      startNewNum = true;
    } else if (k == '=') {
      if (op != ' ') {
        num2 = atof(currentInput);
        if (op == '/' && num2 == 0) {
          hasError = true;
        } else {
          if (op == '+') result = num1 + num2;
          else if (op == '-') result = num1 - num2;
          else if (op == '*') result = num1 * num2;
          else if (op == '/') result = num1 / num2;
          else if (op == '%') result = fmod(num1, num2);

          dtostrf(result, 1, 4, currentInput);
          int l = strlen(currentInput);
          while (l > 1 && currentInput[l - 1] == '0') {
            currentInput[l - 1] = '\0';
            l--;
          }
          if (l > 1 && currentInput[l - 1] == '.') {
            currentInput[l - 1] = '\0';
            l--;
          }
          inputLen = strlen(currentInput);
          num1 = result;
          op = ' ';
          startNewNum = true;
        }
      }
    }
  }

  display.clearDisplay();
  display.drawRect(0, 0, 128, 16, SH110X_WHITE);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  if (hasError) {
    display.setCursor(80, 4);
    display.print("ERROR");
  } else {
    int strLen = strlen(currentInput);
    int xPos = 124 - (strLen * 6);
    if (xPos < 4) xPos = 4;
    display.setCursor(xPos, 4);
    display.print(currentInput);
  }

  int startY = 18;
  int btnWidth = 30;
  int btnHeight = 8;

  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 4; c++) {
      if (r == 4 && c == 3) continue;

      int x = c * 32;
      int y = startY + (r * 9);
      int w = (r == 4 && c == 2) ? 62 : btnWidth;

      bool isSelected = (gridRow == r && gridCol == c);

      if (isSelected) {
        display.fillRect(x, y, w, btnHeight, SH110X_WHITE);
        display.setTextColor(SH110X_BLACK);
      } else {
        display.drawRect(x, y, w, btnHeight, SH110X_WHITE);
        display.setTextColor(SH110X_WHITE);
      }

      display.setCursor(x + (w / 2) - 3, y + 1);
      display.print(keys[r][c]);
    }
  }

  display.display();
}

vvoid runChat(bool up, bool down, bool select) {
  static char messageBuffer[16] = "";
  static int charCount = 0;
  static int currentKeyIndex = 0;
  static bool keyboardActive = true;
  static bool sendingStatus = false;
  static unsigned long sendingTime = 0;
  static String lastResponse = "";

  const char keyboard[] = "ABCDEFGHIJKLMNO"
                           "PQRSTUVWXYZ <-S";
  const int totalKeys = sizeof(keyboard) - 1;
  const int keysPerRow = 15;

  if (sendingStatus && (millis() - sendingTime > 3000)) {
    sendingStatus = false;
  }

  if (selectDoubleClicked) {
    keyboardActive = !keyboardActive;
  }
  else if (select) {
    if (keyboardActive) {
      char selectedChar = keyboard[currentKeyIndex];
      if (selectedChar == 'S') {
        if (charCount > 0) {
          sendingStatus = true;
          sendingTime = millis();

          display.clearDisplay();
          display.setCursor(20, 25);
          display.setTextSize(1);
          display.setTextColor(SH110X_WHITE);
          display.print("Sending...");
          display.display();

          // Construct API URL with user query
          String query = String(messageBuffer);
          query.replace(" ", "%20"); // Simple URL encoding for spaces
          String url = "https://urpocket.libyzxy0.me/api/chat?q=" + query;

          // Fetch API response
          String jsonResponse = fetchHttpsApi(url.c_str());

          // Parse JSON response
          DynamicJsonDocument doc(512);
          DeserializationError error = deserializeJson(doc, jsonResponse);

          if (!error && doc.containsKey("res")) {
            lastResponse = doc["res"].as<String>();
          } else {
            lastResponse = "Error getting response";
          }

          // Reset input buffer & switch to response view
          messageBuffer[0] = '\0';
          charCount = 0;
          keyboardActive = false;
        }
      }
      else if (selectedChar == '<') {
        if (charCount > 0) {
          charCount--;
          messageBuffer[charCount] = '\0';
        }
      } 
      else {
        if (charCount < 15) {
          messageBuffer[charCount] = selectedChar;
          charCount++;
          messageBuffer[charCount] = '\0'; 
        }
      }
    }
  }

  if (keyboardActive && !sendingStatus) {
    if (down) {
      currentKeyIndex += 1;
      if (currentKeyIndex >= totalKeys) currentKeyIndex = 0; 
    }
    if (up) {
      currentKeyIndex -= 1;
      if (currentKeyIndex < 0) currentKeyIndex = totalKeys - 1; 
    }
  }

  display.clearDisplay();

  if (sendingStatus) {
    display.setCursor(20, 25);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.print("Sending...");
    display.display();
    return;
  }

  if (keyboardActive) {
    display.drawRect(0, 2, 128, 24, SH110X_WHITE);
    display.setCursor(4, 10);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    if (charCount == 0) {
      display.print("Type message...");
    } else {
      display.print(messageBuffer);
    }

    for (int i = 0; i < totalKeys; i++) {
      int row = i / keysPerRow;
      int col = i % keysPerRow;
      
      int xPos = col * 8.5 + 2;
      int yPos = 34 + (row * 14);
      if (i == currentKeyIndex) {
        display.fillRect(xPos - 1, yPos - 1, 8, 11, SH110X_WHITE);
        display.setTextColor(SH110X_BLACK);
        display.setCursor(xPos, yPos);
        display.print(keyboard[i]);
      } else {
        display.setTextColor(SH110X_WHITE);
        display.setCursor(xPos, yPos);
        display.print(keyboard[i]);
      }
    }
  } else {
    display.setTextColor(SH110X_WHITE);
    
    const char* textToShow = lastResponse.length() > 0 ? lastResponse.c_str() : "No Message";
    int textLength = strlen(textToShow);

    int textSize = 1;
    if (textLength <= 7) textSize = 3;
    else if (textLength <= 10) textSize = 2;
    
    display.setTextSize(textSize);
    
    if (textSize == 1) {
      display.setTextWrap(true);
      display.setCursor(0, 0);
    } else {
      display.setTextWrap(false);
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds(textToShow, 0, 0, &x1, &y1, &w, &h);
      
      int xPos = (128 - w) / 2 - x1;
      int yPos = (64 - h) / 2 - y1;
      display.setCursor(xPos, yPos);
    }
    
    display.print(textToShow);
  }

  display.display();
}

void runSettings(bool up, bool down, bool select) {
  if (currentSettingsSubState == SETTINGS_MAIN) {
    String wifiLabel = "WiFi  " + String(wifiEnabled ? "            ON" : "           OFF");
    const char* settingsMenuLabels[] = {wifiLabel.c_str(), "Reprogram          >", "TSound             >"};
    const int totalSettingsItems = sizeof(settingsMenuLabels) / sizeof(settingsMenuLabels[0]);

    if (down) {
      settingsSelection++;
      if (settingsSelection >= totalSettingsItems) settingsSelection = 0;
    }
    if (up) {
      settingsSelection--;
      if (settingsSelection < 0) settingsSelection = totalSettingsItems - 1;
    }

    if (select) {
      playClickSound();
      if (settingsSelection == 0) {
        toggleWiFi();
      } else if (settingsSelection == 1) {
        currentSettingsSubState = SETTINGS_OTA;
      } else if (settingsSelection == 2) {
        currentSettingsSubState = SETTINGS_TUNE;
      }
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.print("SETTINGS");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
    for (int i = 0; i < totalSettingsItems; i++) {
      int yPos = 14 + (i * 12);
      if (i == settingsSelection) {
        display.fillRect(0, yPos - 1, SCREEN_WIDTH, 11, SH110X_WHITE);
        display.setTextColor(SH110X_BLACK);
      } else {
        display.setTextColor(SH110X_WHITE);
      }
      display.setCursor(4, yPos);
      display.print(settingsMenuLabels[i]);
    }
    display.display();
  } 
  else if (currentSettingsSubState == SETTINGS_OTA) {
    if (select) {
      playClickSound();
      if (!otaEnabled && wifiEnabled) {
        startOTA();
      } else if (otaEnabled) {
        stopOTA();
      }
    }
    drawOTAUIPage();
  }
  else if (currentSettingsSubState == SETTINGS_TUNE) {
    if (select) {
      playClickSound();
      playHappyBirthday();
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Test Sound");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
    
    display.setCursor(0, 10);
    display.print("Song: Happy Birthday");
    
    display.setCursor(0, 54);
    display.print("Press SEL to Play");
    display.display();
  }
}

void runAlerts(bool up, bool down, bool select) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 20);
  display.setTextSize(1);
  display.print("Alerts App");
  display.setCursor(10, 45);
  display.print("Hold SELECT to exit");
  display.display();
}

void runPowerOff(bool up, bool down, bool select) {
  const char* options[] = {"Power Off", "Restart", "Back"};
  const int totalOptions = 3;

  if (down) {
    powerMenuSelection++;
    if (powerMenuSelection >= totalOptions) powerMenuSelection = 0;
  }
  if (up) {
    powerMenuSelection--;
    if (powerMenuSelection < 0) powerMenuSelection = totalOptions - 1;
  }

  if (select) {
    playClickSound();
    if (powerMenuSelection == 0) {
      turnOffDevice();
      return;
    } else if (powerMenuSelection == 1) {
      ESP.restart();
    } else if (powerMenuSelection == 2) {
      currentState = STATE_MENU;
      drawMenu();
      return;
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("POWER OPTIONS");
   display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  for (int i = 0; i < totalOptions; i++) {
    int yPos = 18 + (i * 15);
    if (i == powerMenuSelection) {
      display.fillRect(0, yPos - 2, SCREEN_WIDTH, 13, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.setTextColor(SH110X_WHITE);
    }
    display.setCursor(4, yPos);
    display.print(options[i]);
  }

  display.display();
}