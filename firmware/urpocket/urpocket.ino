#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h> 
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <esp_system.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define I2C_SDA 3
#define I2C_SCL 4

#define BUZZER_PIN 1
#define LED_PIN 8

#include "icons.h"
#include "AppTypes.h"
#include "AudioSystem.h"
#include "AppNetworkManager.h"
#include "InputController.h"
#include "PomodoroApp.h"
#include "CalendarApp.h"
#include "FlashcardsApp.h"
#include "CalculatorApp.h"
#include "AlertsApp.h"
#include "ChatApp.h"
#include "SettingsApp.h"
#include "PowerApp.h"
#include "URPocketSystem.h"

URPocketSystem systemDevice;

void setup() {
  systemDevice.setup();
}

void loop() {
  systemDevice.loop();
} 