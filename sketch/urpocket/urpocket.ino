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

static const unsigned char PROGMEM icon_calendar[] = {0x09,0x20,0x76,0xdc,0xff,0xfe,0xff,0xfe,0x80,0x02,0x86,0xda,0x86,0xda,0x80,0x02,0xb6,0xda,0xb6,0xda,0x80,0x02,0xb6,0xc2,0xb6,0xc2,0x80,0x02,0x7f,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x04,0x10,0x00,0x00,0x00,0x00,0x00,0x80};
static const unsigned char PROGMEM icon_power[] = {0x01,0x00,0x01,0x00,0x19,0x30,0x25,0x48,0x49,0x24,0x51,0x14,0xa1,0x0a,0xa0,0x0a,0xa0,0x0a,0xa0,0x0a,0xa0,0x0a,0x50,0x14,0x48,0x24,0x27,0xc8,0x18,0x30,0x07,0xc0};
static const unsigned char PROGMEM icon_gear[] = {0x03,0xc0,0x12,0x48,0x2c,0x34,0x40,0x02,0x23,0xc4,0x24,0x24,0xc8,0x13,0x88,0x11,0x88,0x11,0xc8,0x13,0x24,0x24,0x23,0xc4,0x40,0x02,0x2c,0x34,0x12,0x48,0x03,0xc0};
static const unsigned char PROGMEM icon_message[] = {0x7f,0xfe,0x80,0x02,0x80,0x01,0x96,0xd1,0x9f,0xf9,0x80,0x01,0x80,0x01,0x95,0xb1,0x9f,0xf9,0x80,0x01,0x80,0x01,0x63,0xfe,0x14,0x00,0x08,0x00,0x10,0x00,0x00,0x00};
static const unsigned char PROGMEM icon_flashcards[] = {0x7f,0xfc,0xc0,0x06,0xff,0xf2,0x80,0x0a,0x80,0x0a,0x9f,0xea,0x90,0x2a,0x9f,0xea,0x80,0x0a,0x80,0x0a,0x8f,0x8a,0x80,0x0a,0x80,0x0a,0x8f,0x8a,0x80,0x0c,0x7f,0xf8};
static const unsigned char PROGMEM icon_bell[] = {0x11,0x88,0x63,0xc6,0x44,0x22,0x8c,0x11,0x88,0x11,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x20,0x04,0x20,0x04,0x40,0x02,0xff,0xff,0x06,0x60,0x03,0xc0,0x00,0x00};
static const unsigned char PROGMEM icon_pomodoro[] = {0xff,0xe0,0x40,0x40,0x40,0x40,0x51,0x40,0x5f,0x40,0x2e,0x80,0x15,0x00,0x0a,0x00,0x0a,0x00,0x11,0x00,0x24,0x80,0x44,0x40,0x4e,0x40,0x5f,0x40,0x7f,0xc0,0xff,0xe0};
static const unsigned char PROGMEM icon_calculator[] = {0x7f,0xe0,0x80,0x10,0xbf,0xd0,0xa0,0x50,0xbf,0xd0,0x80,0x10,0xb6,0xd0,0xb6,0xd0,0x80,0x10,0xb6,0xd0,0xb6,0xd0,0x80,0x10,0xb6,0xd0,0xb6,0xd0,0x80,0x10,0x7f,0xe0};
static const unsigned char PROGMEM image_device_power_button_bits[] = {0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x03,0xc3,0x0f,0x00,0x03,0xc3,0x0f,0x00,0x0c,0x33,0x30,0xc0,0x0c,0x33,0x30,0xc0,0x30,0xc3,0x0c,0x30,0x30,0xc3,0x0c,0x30,0x33,0x03,0x03,0x30,0x33,0x03,0x03,0x30,0xcc,0x03,0x00,0xcc,0xcc,0x03,0x00,0xcc,0xcc,0x00,0x00,0xcc,0xcc,0x00,0x00,0xcc,0xcc,0x00,0x00,0xcc,0xcc,0x00,0x00,0xcc,0xcc,0x00,0x00,0xcc,0xcc,0x00,0x00,0xcc,0xcc,0x00,0x00,0xcc,0xcc,0x00,0x00,0xcc,0x33,0x00,0x03,0x30,0x33,0x00,0x03,0x30,0x30,0xc0,0x0c,0x30,0x30,0xc0,0x0c,0x30,0x0c,0x3f,0xf0,0xc0,0x0c,0x3f,0xf0,0xc0,0x03,0xc0,0x0f,0x00,0x03,0xc0,0x0f,0x00,0x00,0x3f,0xf0,0x00,0x00,0x3f,0xf0,0x00};
static const unsigned char PROGMEM image_device_reset_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0xff,0xc0,0x00,0x30,0xff,0xc0,0x03,0xc0,0xfc,0x00,0x03,0xc0,0xfc,0x00,0x0c,0x00,0xfc,0x00,0x0c,0x00,0xfc,0x00,0x30,0x00,0xc3,0x00,0x30,0x00,0xc3,0x00,0x30,0x00,0xc3,0x00,0x30,0x00,0xc3,0x00,0xc0,0x00,0x00,0xc0,0xc0,0x00,0x00,0xc0,0xc0,0x00,0x00,0xc0,0xc0,0x00,0x00,0xc0,0xc0,0x00,0x00,0xc0,0xc0,0x00,0x00,0xc0,0x30,0x00,0x03,0x00,0x30,0x00,0x03,0x00,0x30,0x00,0x03,0x00,0x30,0x00,0x03,0x00,0x0c,0x00,0x0c,0x00,0x0c,0x00,0x0c,0x00,0x03,0xc0,0xf0,0x00,0x03,0xc0,0xf0,0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM image_space[] = {0x1f,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x00};
static const unsigned char PROGMEM image_logo[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xf8,0x00,0x00,0x00,0x00,0x00,0x7f,0xc0,0x1c,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x02,0x00,0x00,0x00,0x00,0x01,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xf8,0x03,0xfe,0x00,0x00,0x00,0x00,0x07,0xf0,0x1f,0xff,0x80,0x00,0x00,0x00,0x07,0xe0,0x7f,0xff,0xe0,0x00,0x00,0x00,0x07,0xc0,0xff,0xff,0xf0,0x00,0x00,0x00,0x0f,0x81,0xff,0x80,0x3c,0x00,0x00,0x00,0x0f,0x83,0xfc,0x00,0x04,0x00,0x00,0x00,0x0f,0x07,0xf8,0x00,0x02,0x00,0x00,0x00,0x0f,0x07,0xe0,0x0e,0x00,0x00,0x00,0x00,0x0f,0x0f,0xc0,0xff,0xe0,0x00,0x00,0x00,0x1e,0x0f,0x83,0xff,0xf0,0x00,0x00,0x00,0x0e,0x1f,0x87,0xff,0xfc,0x00,0x00,0x00,0x0e,0x1f,0x0f,0xff,0xfe,0x00,0x00,0x00,0x0e,0x1e,0x0f,0xff,0xfe,0x00,0x00,0x00,0x0e,0x3e,0x1f,0xff,0xff,0x00,0x00,0x00,0x0e,0x3e,0x3f,0xff,0xff,0x00,0x00,0x00,0x06,0x3c,0x3f,0xff,0xff,0x80,0x00,0x00,0x07,0x3c,0x3f,0xff,0xff,0x80,0x00,0x00,0x03,0x3c,0x7f,0xff,0xff,0x80,0x00,0x00,0x03,0x3c,0x7f,0xff,0xff,0xc0,0x00,0x00,0x01,0x9c,0x7f,0xff,0xff,0xc0,0x00,0x00,0x00,0x9c,0x7f,0xff,0xff,0xc0,0x00,0x00,0x00,0x5c,0x7f,0xff,0xff,0x80,0x00,0x00,0x00,0x0c,0x3f,0xff,0xff,0x80,0x00,0x00,0x00,0x0c,0x3f,0xff,0xff,0x80,0x00,0x00,0x00,0x06,0x3f,0xff,0xff,0x00,0x00,0x00,0x00,0x06,0x1f,0xff,0xff,0x00,0x00,0x00,0x00,0x02,0x1f,0xff,0xfe,0x00,0x00,0x00,0x00,0x01,0x0f,0xff,0xfe,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xfc,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM image_certification[] = {0xfc,0x7f,0x8f,0xc0,0x00,0x3c,0x00,0x07,0x80,0x00,0x00,0x3f,0x80,0xfc,0x7f,0x8f,0xc0,0x00,0xfc,0x00,0x1f,0x80,0x1f,0xf0,0xff,0xe0,0xfc,0x73,0x8f,0xc0,0x03,0xfc,0x00,0x7f,0x80,0x1f,0xe3,0xc0,0x78,0xe0,0x73,0x8e,0x00,0x07,0xe0,0x00,0xfc,0x00,0x18,0x07,0x00,0x1c,0xe0,0x73,0x8e,0x00,0x0f,0x80,0x01,0xf0,0x00,0x18,0x0e,0x00,0x0e,0xe0,0x73,0x8e,0x00,0x1e,0x00,0x03,0xc0,0x00,0x18,0x0c,0x1f,0x00,0xe0,0x73,0x8e,0x00,0x1c,0x00,0x03,0x80,0x00,0x18,0x18,0x7f,0xc0,0xe0,0x73,0x8e,0x00,0x3c,0x00,0x07,0x80,0x00,0x18,0x18,0xe0,0xe0,0xe0,0x73,0x8e,0x00,0x38,0x00,0x07,0x00,0x00,0x18,0x30,0xc0,0x40,0xe0,0x73,0x8e,0x00,0x38,0x00,0x07,0x00,0x00,0x18,0x31,0x80,0x00,0xfc,0x7f,0x8e,0x00,0x38,0x00,0x07,0xfe,0x00,0x1f,0xf1,0x80,0x00,0xfc,0x7f,0x8e,0x00,0x38,0x00,0x07,0xfe,0x00,0x1f,0xf1,0x80,0x00,0xfc,0x7f,0x8e,0x00,0x38,0x00,0x07,0xfe,0x00,0x18,0x31,0x80,0x00,0xe0,0x73,0x8e,0x00,0x38,0x00,0x07,0x00,0x00,0x18,0x31,0x80,0x00,0xe0,0x73,0x8e,0x00,0x38,0x00,0x07,0x00,0x00,0x18,0x30,0xc0,0x40,0xe0,0x73,0x8e,0x00,0x3c,0x00,0x07,0x80,0x00,0x18,0x18,0xe0,0xe0,0xe0,0x73,0x8e,0x00,0x1c,0x00,0x03,0x80,0x00,0x18,0x18,0x7f,0xc0,0xe0,0x73,0x8e,0x00,0x1e,0x00,0x03,0xc0,0x00,0x18,0x0c,0x1f,0x06,0xe0,0x73,0x8e,0x00,0x0f,0x80,0x01,0xf0,0x00,0x18,0x0e,0x00,0x0e,0xe0,0x73,0x8e,0x00,0x07,0xe0,0x00,0xfc,0x00,0x18,0x07,0x00,0x1c,0xfc,0x73,0x8f,0xc0,0x03,0xfc,0x00,0x7f,0x80,0x18,0x03,0xc0,0x78,0xfc,0x73,0x8f,0xc0,0x00,0xfc,0x00,0x1f,0x80,0x10,0x00,0xff,0xe0,0xfc,0x73,0x8f,0xc0,0x00,0x3c,0x00,0x07,0x80,0x00,0x00,0x3f,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x33,0x07,0x00,0x80,0x00,0x02,0x00,0x00,0x07,0x00,0x00,0x00,0x30,0x33,0x0e,0x00,0x41,0xff,0x04,0x00,0x00,0x0f,0x80,0x00,0x00,0x30,0x33,0x3c,0x00,0x26,0x00,0xc8,0x00,0x00,0x1f,0xc0,0x00,0x00,0x30,0x33,0x70,0x00,0x18,0x00,0x30,0x00,0x00,0x38,0xe0,0x00,0x00,0x30,0x33,0xe0,0x00,0x0f,0xff,0xe0,0x00,0x00,0x30,0x60,0x00,0x00,0x30,0x33,0xe0,0x00,0x04,0x00,0x40,0x00,0x00,0x70,0x70,0x00,0x00,0x30,0x33,0x70,0x00,0x06,0x00,0xc0,0x00,0x00,0xe0,0x38,0x00,0x00,0x30,0x33,0x3c,0x00,0x05,0x01,0x40,0x00,0x01,0xcf,0x9c,0x00,0x00,0x3f,0xf3,0x0e,0x00,0x04,0xbe,0x40,0x00,0x01,0x90,0x4c,0x00,0x00,0x1f,0xe3,0x07,0x00,0x04,0x44,0x40,0x00,0x03,0xa0,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x28,0x80,0x00,0x07,0x40,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x10,0x80,0x00,0x0e,0x80,0x2b,0x80,0x00,0x1f,0xf1,0xfe,0x00,0x02,0x28,0x80,0x00,0x0c,0x88,0x49,0x80,0x00,0x3f,0xf3,0xff,0x00,0x02,0x44,0x80,0x00,0x1c,0x88,0x89,0xc0,0x00,0x30,0x03,0x03,0x00,0x02,0x82,0x80,0x00,0x38,0x85,0x08,0xe0,0x00,0x30,0x03,0x03,0x00,0x01,0x01,0x00,0x00,0x70,0x86,0x08,0x70,0x00,0x30,0x03,0x03,0x00,0x03,0x01,0x80,0x00,0x60,0x40,0x10,0x30,0x00,0x30,0x03,0xff,0x00,0x05,0x03,0xc0,0x00,0xe0,0x20,0x20,0x38,0x00,0x30,0x03,0xff,0x00,0x09,0x04,0x60,0x01,0xc0,0x10,0x40,0x1c,0x00,0x30,0x03,0x03,0x00,0x11,0x05,0x50,0x01,0x80,0x0f,0x80,0x0c,0x00,0x3f,0xf3,0x03,0x00,0x20,0xfc,0x48,0x01,0xc0,0x00,0x00,0x1c,0x00,0x1f,0xf3,0x03,0x00,0x40,0x03,0x84,0x00,0xff,0xff,0xff,0xf8,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x02,0x00,0x7f,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xf8,0x00,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM image_Pin_arrow_right_bits[] = {0x04, 0x00, 0x06, 0x00, 0xff, 0x00, 0xff, 0x80, 0xff, 0x00, 0x06, 0x00, 0x04, 0x00};
static const unsigned char PROGMEM image_SmallArrowUp_bits[] = {0x10, 0x38, 0x7c, 0xfe};
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

class AudioSystem {
private:
  uint8_t pin;
public:
  AudioSystem(uint8_t buzzerPin) : pin(buzzerPin) {}
  void playClickSound() {
    tone(pin, 1800, 15);
  }
  void playBootSound() {
    tone(pin, 1047, 100); delay(120);
    tone(pin, 1318, 100); delay(120);
    tone(pin, 1568, 100); delay(120);
    tone(pin, 2093, 200); delay(200);
  }
  void playPowerSound() {
    tone(pin, 2093, 100); delay(120);
    tone(pin, 1568, 100); delay(120);
    tone(pin, 1318, 100); delay(120);
    tone(pin, 1047, 200); delay(200);
  }
  void playHappyBirthday() {
    int melody[] = {
      262, 262, 294, 262, 349, 330,
      262, 262, 294, 262, 392, 349,
      262, 262, 523, 440, 349, 330, 294,
      466, 466, 440, 349, 392, 349
    };
    int noteDurations[] = {
      4, 4, 2, 2, 2, 1,
      4, 4, 2, 2, 2, 1,
      4, 4, 2, 2, 2, 2, 1,
      4, 4, 2, 2, 2, 1
    };
    for (int thisNote = 0; thisNote < 25; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(pin, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(pin);
    }
  }
};

class AppNetworkManager {
private:
  const char* ssid = "OrangeCat";
  const char* password = "myorange32";
  const char* ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = 8 * 3600;
  const int daylightOffset_sec = 0;
  bool wifiEnabled = false;

public:
  bool isWifiEnabled() const { return wifiEnabled; }
  void toggleWiFi() {
    wifiEnabled = !wifiEnabled;
    if (wifiEnabled) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    } else {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }
  }
  String fetchHttpsApi(const char* url) {
    if (WiFi.status() != WL_CONNECTED) {
      return "Error: No WiFi";
    }
    WiFiClientSecure client;
    client.setInsecure();
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
};

class InputController {
private:
  int pinUp, pinDown, pinSelect;
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

public:
  InputController(int up, int down, int select)
    : pinUp(up), pinDown(down), pinSelect(select) {}

  void begin() {
    pinMode(pinUp, INPUT_PULLUP);
    pinMode(pinDown, INPUT_PULLUP);
    pinMode(pinSelect, INPUT_PULLUP);
  }

  void update(bool &upPressed, bool &downPressed, bool &selectPressed, bool &selectLongPressed, bool &selectDoubleClicked) {
    upPressed = false;
    downPressed = false;
    selectPressed = false;
    selectLongPressed = false;
    selectDoubleClicked = false;

    if (selectClickPending && (millis() - lastSelectReleaseTime > doubleClickDelay)) {
      selectPressed = true;
      selectClickPending = false;
    }

    if (millis() - lastDebounceTime > debounceDelay) {
      bool upState = digitalRead(pinUp);
      bool downState = digitalRead(pinDown);
      bool selectState = digitalRead(pinSelect);

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
  }
};

class PomodoroApp {
private:
  unsigned long lastTimerUpdate = 0;
  int secondsLeft = 1500;
public:
  void run(Adafruit_SH1106G &display, bool up, bool down, bool select) {
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
};

class CalendarApp {
private:
  struct LocalEvent {
    String summary;
    String code;
    String startTime;
    String timeRange;
    String location;
  };
  bool calendarLoaded = false;
  bool viewingDetail = false;
  int currentEventIndex = 0;
  int totalEvents = 0;
  LocalEvent events[5];

public:
  void run(Adafruit_SH1106G &display, AppNetworkManager &net, AudioSystem &audio, bool up, bool down, bool select) {
    if (!calendarLoaded) {
      display.clearDisplay();
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);
      display.setCursor(10, 25);
      display.print("Fetching Schedule...");
      display.display();

      String jsonResponse = net.fetchHttpsApi("https://urpocket.libyzxy0.me/api/calendar/today");
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, jsonResponse);

      totalEvents = 0;
      if (!error && doc.is<JsonArray>()) {
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject obj : array) {
          if (totalEvents >= 5) break;
          events[totalEvents].summary = obj["summary"] | "No Summary";
          events[totalEvents].code = obj["code"] | "";
          events[totalEvents].startTime = obj["startTime"] | "";
          events[totalEvents].timeRange = obj["timeRange"] | "";
          events[totalEvents].location = obj["location"] | "";
          totalEvents++;
        }
      }
      calendarLoaded = true;
    }

    if (viewingDetail) {
      if (up || down || select) {
        audio.playClickSound();
        viewingDetail = false;
      }
    } else {
      if (down && totalEvents > 0) {
        audio.playClickSound();
        currentEventIndex = (currentEventIndex + 1) % totalEvents;
      }
      if (up && totalEvents > 0) {
        audio.playClickSound();
        currentEventIndex = (currentEventIndex - 1 + totalEvents) % totalEvents;
      }
      if (select && totalEvents > 0) {
        audio.playClickSound();
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
        display.print(events[i].startTime);
        display.print(" ");
        display.print(events[i].code);
      }
    }
    display.display();
  }
};

class FlashcardsApp {
private:
  int flashcardIndex = 0;
  bool showAnswer = false;
  const char* flashcards[3][2] = {
    {"What is OOP?", "Object Oriented Programming"},
    {"ESP32 C3 Core?", "RISC-V Single Core"},
    {"Default I2C SDA?", "GPIO 3"}
  };
public:
  void run(Adafruit_SH1106G &display, AudioSystem &audio, bool up, bool down, bool select) {
    if (select) {
      audio.playClickSound();
      showAnswer = !showAnswer;
    }
    if (down) {
      audio.playClickSound();
      flashcardIndex = (flashcardIndex + 1) % 3;
      showAnswer = false;
    }
    if (up) {
      audio.playClickSound();
      flashcardIndex = (flashcardIndex - 1 + 3) % 3;
      showAnswer = false;
    }
    display.clearDisplay();
    display.setCursor(10, 10);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.print("FLASHCARD ");
    display.print(flashcardIndex + 1);
    display.print("/3");
    display.setCursor(10, 30);
    if (showAnswer) {
      display.print(flashcards[flashcardIndex][1]);
    } else {
      display.print(flashcards[flashcardIndex][0]);
    }
    display.setCursor(10, 52);
    display.print("SEL: Flip | UP/DN: Next");
    display.display();
  }
};

class CalculatorApp {
private:
  char currentInput[16] = "0";
  int inputLen = 1;
  double num1 = 0;
  double num2 = 0;
  double result = 0;
  char op = ' ';
  bool startNewNum = true;
  bool hasDecimal = false;
  bool hasError = false;
  int cursorIndex = 0;
  int gridRow = 0;
  int gridCol = 0;

  const char keys[5][4] = {
    {'C', '<', '%', '/'},
    {'7', '8', '9', '*'},
    {'4', '5', '6', '-'},
    {'1', '2', '3', '+'},
    {'0', '.', '=', ' '}
  };

public:
  void run(Adafruit_SH1106G &display, AudioSystem &audio, bool up, bool down, bool select) {
    if (down) {
      audio.playClickSound();
      cursorIndex++;
      if (cursorIndex > 18) cursorIndex = 0;
    }
    if (up) {
      audio.playClickSound();
      cursorIndex--;
      if (cursorIndex < 0) cursorIndex = 18;
    }

    if (cursorIndex < 16) {
      gridRow = cursorIndex / 4;
      gridCol = cursorIndex % 4;
    } else if (cursorIndex == 16) {
      gridRow = 4; gridCol = 0;
    } else if (cursorIndex == 17) {
      gridRow = 4; gridCol = 1;
    } else if (cursorIndex == 18) {
      gridRow = 4; gridCol = 2;
    }

    if (gridRow == 4 && gridCol == 3) {
      cursorIndex = 18;
      gridRow = 4;
      gridCol = 2;
    }

    if (select) {
      audio.playClickSound();
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
        startNewNum = true;
        hasDecimal = false;
        hasError = false;
      } else if (k == '+' || k == '-' || k == '*' || k == '/' || k == '%') {
        if (op != ' ' && !startNewNum) {
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

        bool isSelected = (r == gridRow && c == gridCol);

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
};

class AlertsApp {
private:
  int alertIndex = 0;
  const char* alerts[3] = {
    "Math Exam tomorrow!",
    "Submit Project on Fri",
    "Drink water!"
  };
public:
  void run(Adafruit_SH1106G &display, AudioSystem &audio, bool up, bool down, bool select) {
    if (down) {
      audio.playClickSound();
      alertIndex = (alertIndex + 1) % 3;
    }
    if (up) {
      audio.playClickSound();
      alertIndex = (alertIndex - 1 + 3) % 3;
    }
    display.clearDisplay();
    display.setCursor(10, 10);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.print("ALERTS & REMINDERS");
    display.setCursor(10, 30);
    display.print(alerts[alertIndex]);
    display.setCursor(10, 52);
    display.print("UP/DN: Navigate");
    display.display();
  }
};

class ChatApp {
private:
  char messageBuffer[64] = "";
  int charCount = 0;
  int currentKeyIndex = 0;
  bool keyboardActive = false;
  bool sendingStatus = false;
  bool isShifted = false;
  unsigned long sendingTime = 0;
  String lastResponse = "";

  static constexpr int totalKeys = 50;
  static constexpr int standardKeys = 46; 
  static constexpr int keysPerRow = 10;

  static constexpr char keyboardUnshifted[standardKeys + 1] = "1234567890QWERTYUIOPASDFGHJKL.ZXCVBNM?!@#,;:-+";
  static constexpr char keyboardShifted[standardKeys + 1]   = "1234567890qwertyuiopasdfghjkl#zxcvbnm$%*=/()&'";

public:
  void run(Adafruit_SH1106G &display, AppNetworkManager &net, AudioSystem &audio, bool up, bool down, bool select, bool selectDoubleClicked) {
    if (sendingStatus && (millis() - sendingTime > 10000)) {
      sendingStatus = false;
    }
    if (!keyboardActive && !sendingStatus) {
      if (up || down || select) {
        audio.playClickSound();
        keyboardActive = true;
        currentKeyIndex = 0;
        messageBuffer[0] = '\0';
        charCount = 0;
        isShifted = false;
        return;
      }
    }

    if (keyboardActive && !sendingStatus) {
      if (select) {
        audio.playClickSound();

        if (currentKeyIndex == 49) {
          if (charCount > 0) {
            sendingStatus = true;
            sendingTime = millis();
            
            display.clearDisplay();
            display.setCursor(20, 25);
            display.setTextSize(1);
            display.setTextColor(SH110X_WHITE);
            display.print("Sending...");
            display.display();

            String query = String(messageBuffer);
            query.replace(" ", "%20");
            String url = "https://urpocket.libyzxy0.me/api/chat?q=" + query;
            String jsonResponse = net.fetchHttpsApi(url.c_str());

            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, jsonResponse);

            if (!error && doc.containsKey("res")) {
              lastResponse = doc["res"].as<String>();
            } else {
              lastResponse = "Error getting response";
            }

            messageBuffer[0] = '\0';
            charCount = 0;
            keyboardActive = false;
            sendingStatus = false;
            return;
          }
        } 

        else if (currentKeyIndex == 47) {
          isShifted = !isShifted;
        } 

        else if (currentKeyIndex == 48) {
          if (charCount > 0) {
            charCount--;
            messageBuffer[charCount] = '\0';
          }
        } 
        else if (currentKeyIndex == 46) {
          if (charCount < (int)sizeof(messageBuffer) - 1) {
            messageBuffer[charCount] = ' ';
            charCount++;
            messageBuffer[charCount] = '\0';
          }
        } 
        else if (currentKeyIndex < standardKeys) {
          const char* activeKeyboard = isShifted ? keyboardShifted : keyboardUnshifted;
          char selectedChar = activeKeyboard[currentKeyIndex];

          if (charCount < (int)sizeof(messageBuffer) - 1) {
            messageBuffer[charCount] = selectedChar;
            charCount++;
            messageBuffer[charCount] = '\0';
          }
        }
      }

      if (down) {
        audio.playClickSound();
        currentKeyIndex = (currentKeyIndex + 1) % totalKeys;
      }
      if (up) {
        audio.playClickSound();
        currentKeyIndex = (currentKeyIndex - 1 + totalKeys) % totalKeys;
      }
    }

    display.clearDisplay();

    if (sendingStatus) {
      display.setCursor(0, 24);
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      display.print("Processing, please wait...");
      display.display();
      return;
    }

    if (keyboardActive) {
      const char* activeKeyboard = isShifted ? keyboardShifted : keyboardUnshifted;

      display.drawRect(0, 0, 128, 13, SH110X_WHITE);
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);

      int maxVisibleChars = 19;
      int startIndex = (charCount > maxVisibleChars) ? (charCount - maxVisibleChars) : 0;
      
      display.setCursor(3, 3);
      display.print(&messageBuffer[startIndex]);

      if ((millis() % 1000) < 500) {
        display.print("_");
      }
      int keyWidth = 12;
      int keyHeight = 9;
      int startY = 16;
      int startX = 4;

      for (int i = 0; i < totalKeys; i++) {
        int row = i / keysPerRow;
        int col = i % keysPerRow;

        int xPos = startX + (col * keyWidth);
        int yPos = startY + (row * keyHeight);

        bool isSelected = (i == currentKeyIndex);
        bool isShiftButton = (i == 47);

        if (isSelected || (isShiftButton && isShifted)) {
          display.fillRect(xPos - 1, yPos - 1, keyWidth - 1, keyHeight - 1, SH110X_WHITE);
          display.setTextColor(SH110X_BLACK);
        } else {
          display.setTextColor(SH110X_WHITE);
        }

        display.setCursor(xPos + 1, yPos);

        if (i == 46) {
          display.print(" ");
        } else if (i == 47) {
          int color = (isSelected || isShifted) ? SH110X_BLACK : SH110X_WHITE;
          display.drawBitmap(xPos + 2, yPos + 2, image_SmallArrowUp_bits, 8, 4, color);
        } else if (i == 48) {
          display.print("<");
        } else if (i == 49) {
          int color = isSelected ? SH110X_BLACK : SH110X_WHITE;
          display.drawBitmap(xPos + 1, yPos + 1, image_Pin_arrow_right_bits, 9, 7, color);
        } else {
          display.print(activeKeyboard[i]);
        }
      }
    } else {
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("ORENS AI");
      display.drawLine(0, 10, 128, 10, SH110X_WHITE);
      display.setCursor(0, 14);
      display.setTextWrap(true);
      display.print(lastResponse.length() > 0 ? lastResponse : "Press any key to type...");
    }

    display.display();
  }
};

class SettingsApp {
private:
  SettingsMenu currentSettingsSubState = SETTINGS_MAIN;
  int settingsSelection = 0;
  int sysInfoPage = 0;

  void drawOTAUIPage(Adafruit_SH1106G &display, bool updating, int progressPercent, const char* statusMsg, IPAddress ip) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("OTA UPDATES");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);

    display.setCursor(0, 16);
    display.print("ESP32C3 Dev Module");

    if (WiFi.status() == WL_CONNECTED) {
      display.setCursor(0, 26);
      display.print(ip);
    }

    if (updating) {
      static unsigned long lastBlinkTime = 0;
      if (millis() - lastBlinkTime >= 100) {
        lastBlinkTime = millis();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      }

      display.drawRect(0, 40, 128, 12, SH110X_WHITE);
      int fillWidth = map(progressPercent, 0, 100, 0, 124);
      display.fillRect(2, 42, fillWidth, 8, SH110X_WHITE);

      display.setCursor(0, 54);
      display.print("Progress");

      if(progressPercent >= 100) {
        display.setCursor(104, 54);
      } else if (progressPercent >= 10) {
        display.setCursor(110, 54);
      } else {
        display.setCursor(115, 54);
      }
      display.print(progressPercent);
      display.print("%");
    } else {
      static unsigned long lastBlinkTime = 0;
      if (millis() - lastBlinkTime >= 1500) {
        lastBlinkTime = millis();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      }
      display.setCursor(0, 50);
      display.print("> ");
      display.print(statusMsg);
    }
    display.display();
  }

  void setupOTAHandlers(Adafruit_SH1106G &display) {
    ArduinoOTA.onStart([this, &display]() {
      drawOTAUIPage(display, true, 0, "Updating Firmware...", WiFi.localIP());
    });

    ArduinoOTA.setPassword("libyzxy0");

    ArduinoOTA.onEnd([this, &display]() {
      drawOTAUIPage(display, false, 100, "Update Complete!", WiFi.localIP());
      delay(1000);
    });

    ArduinoOTA.onProgress([this, &display](unsigned int progress, unsigned int total) {
      int pct = (total > 0) ? (progress * 100UL / total) : 0;
      drawOTAUIPage(display, true, pct, "Updating Firmware...", WiFi.localIP());
    });

    ArduinoOTA.onError([this, &display](ota_error_t error) {
      const char* msg;
      switch(error) {
        case OTA_AUTH_ERROR:    msg = "Auth Failed"; break;
        case OTA_BEGIN_ERROR:   msg = "Begin Failed"; break;
        case OTA_CONNECT_ERROR: msg = "Connect Failed"; break;
        case OTA_RECEIVE_ERROR: msg = "Receive Failed"; break;
        case OTA_END_ERROR:     msg = "End Failed"; break;
        default:                msg = "Update Error"; break;
      }
      drawOTAUIPage(display, false, 0, msg, WiFi.localIP());
      delay(2000);
    });

    ArduinoOTA.begin();
  }

public:
  SettingsMenu getSubState() const { return currentSettingsSubState; }
  void resetSubState() { currentSettingsSubState = SETTINGS_MAIN; }

  void run(Adafruit_SH1106G &display, AppNetworkManager &net, AudioSystem &audio, AppState &currentState, bool up, bool down, bool select) {
    if (currentSettingsSubState == SETTINGS_MAIN) {
      ArduinoOTA.end();

      const char* options[] = {"WiFi", "OTA Update", "Play Tune", "System Info"};
      const char* indicators[] = {">", ">", "#", ">"};
      const int totalOptions = 4;

      if (down) {
        settingsSelection++;
        if (settingsSelection >= totalOptions) settingsSelection = 0;
      }
      if (up) {
        settingsSelection--;
        if (settingsSelection < 0) settingsSelection = totalOptions - 1;
      }

      if (select) {
        audio.playClickSound();
        if (settingsSelection == 0) {
          currentSettingsSubState = SETTINGS_WIFI;
        } else if (settingsSelection == 1) {
          currentSettingsSubState = SETTINGS_OTA;
        } else if (settingsSelection == 2) {
          currentSettingsSubState = SETTINGS_TUNE;
          audio.playHappyBirthday();
          currentSettingsSubState = SETTINGS_MAIN;
        } else if (settingsSelection == 3) {
          currentSettingsSubState = SETTINGS_SYSINFO;
        } else if (settingsSelection == 4) {
          currentState = STATE_MENU;
          return;
        }
      }

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      display.setCursor(0, 0);
      display.print("SETTINGS");
      display.drawLine(0, 10, 128, 10, SH110X_WHITE);

      for (int i = 0; i < totalOptions; i++) {
        int y = 14 + (i * 10);
        if (i == settingsSelection) {
          display.fillRect(0, y - 1, 128, 9, SH110X_WHITE);
          display.setTextColor(SH110X_BLACK);
        } else {
          display.setTextColor(SH110X_WHITE);
        }

        display.setCursor(2, y);
        display.print(options[i]);

        if (indicators[i][0] != '\0') {
          display.setCursor(120, y);
          display.print(indicators[i]);
        }
      }
      display.display();

    } else if (currentSettingsSubState == SETTINGS_WIFI) {
      if (select) {
        audio.playClickSound();
        net.toggleWiFi();
      }
      if (up || down) {
        audio.playClickSound();
        currentSettingsSubState = SETTINGS_MAIN;
      }

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      display.setCursor(0, 0);
      display.print("WIFI SETTINGS");
      display.drawLine(0, 10, 128, 10, SH110X_WHITE);

      display.setCursor(0, 16);
      display.print("Status: ");
      display.print(net.isWifiEnabled() ? "ENABLED" : "DISABLED");

      display.setCursor(0, 28);
      display.print("Conn: ");
      if (WiFi.status() == WL_CONNECTED) {
        display.print("Connected");
        display.setCursor(0, 40);
        display.print("IP: ");
        display.print(WiFi.localIP());
      } else if (net.isWifiEnabled()) {
        display.print("Connecting...");
      } else {
        display.print("Disconnected");
      }

      display.setCursor(0, 54);
      display.print("SEL: Toggle | UP/DN: Back");
      display.display();

    } else if (currentSettingsSubState == SETTINGS_OTA) {
      static bool otaStarted = false;

      if (!net.isWifiEnabled() || WiFi.status() != WL_CONNECTED) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(0, 0);
        display.print("OTA UPDATES");
        display.drawLine(0, 10, 128, 10, SH110X_WHITE);

        display.setCursor(0, 20);
        display.print("Error: WiFi Not");
        display.setCursor(0, 30);
        display.print("Connected!");

        display.setCursor(0, 50);
        display.print("Press SEL to Back");
        display.display();

        if (select || up || down) {
          audio.playClickSound();
          currentSettingsSubState = SETTINGS_MAIN;
          otaStarted = false;
        }
        return;
      }

      if (!otaStarted) {
        setupOTAHandlers(display);
        otaStarted = true;
      }

      ArduinoOTA.handle();
      drawOTAUIPage(display, false, 0, "Ready for OTA...", WiFi.localIP());

      if (up || down) {
        audio.playClickSound();
        ArduinoOTA.end();
        otaStarted = false;
        currentSettingsSubState = SETTINGS_MAIN;
      }

    } else if (currentSettingsSubState == SETTINGS_SYSINFO) {
      if (select) {
        audio.playClickSound();
        sysInfoPage = (sysInfoPage + 1) % 2;
      }
      if (up || down) {
        audio.playClickSound();
        currentSettingsSubState = SETTINGS_MAIN;
      }

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      display.setCursor(0, 0);
      display.print("SYSTEM INFO (");
      display.print(sysInfoPage + 1);
      display.print("/2)");
      display.drawLine(0, 10, 128, 10, SH110X_WHITE);

      if (sysInfoPage == 0) {
        display.setCursor(0, 14);
        display.print("OS: UrPocket OS");
        display.setCursor(0, 24);
        display.print("Ver: v1.0.0");
        display.setCursor(0, 34);
        display.print("SDK: ");
        display.print(ESP.getSdkVersion());
        display.setCursor(0, 44);
        display.print("Chip: ");
        display.print(ESP.getChipModel());
        display.print(" ");
        display.print(ESP.getChipRevision());
        display.setCursor(0, 54);
        display.print("MAC:");
        display.print(WiFi.macAddress());
      } else {
        display.setCursor(0, 14);
        display.print("Heap: ");
        display.print(ESP.getFreeHeap() / 1024);
        display.print(" KB");
        display.setCursor(0, 24);
        display.print("CPU Freq: ");
        display.print(ESP.getCpuFreqMHz());
        display.print(" MHz ");
        display.print(ESP.getChipCores());
        display.print(" core");
        display.setCursor(0, 34);
        display.print("Temp: ");
        display.print(temperatureRead(), 1);
        display.print(" C");
        display.setCursor(0, 44);
        display.print("Uptime: ");
        display.print(millis() / 1000);
        display.print("s");
        display.setCursor(0, 54);
        display.print("Flash: ");
        display.print(ESP.getFlashChipSize() / (1024 * 1024));
        display.print(" MB");
      }
      display.display();
    }
  }
};

class PowerApp {
private:
  int powerMenuSelection = 0;

  void turnOffDevice(Adafruit_SH1106G &display, AudioSystem &audio) {
    audio.playPowerSound();
    display.clearDisplay();
    display.display();
    digitalWrite(LED_PIN, LOW);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << 5, ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_deep_sleep_start();
  }

public:
  void run(Adafruit_SH1106G &display, AudioSystem &audio, bool up, bool down, bool select) {
    const int totalOptions = 2;

    if (down) {
      powerMenuSelection++;
      if (powerMenuSelection >= totalOptions) powerMenuSelection = 0;
    }
    if (up) {
      powerMenuSelection--;
      if (powerMenuSelection < 0) powerMenuSelection = totalOptions - 1;
    }

    if (select) {
      if (powerMenuSelection == 0) {
        turnOffDevice(display, audio);
        return;
      } else if (powerMenuSelection == 1) {
        ESP.restart();
        return;
      }
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextWrap(false);

    if (powerMenuSelection == 0) {
      display.fillRoundRect(2, 4, 58, 56, 4, SH110X_WHITE);
      display.drawBitmap(16, 9, image_device_power_button_bits, 30, 32, SH110X_BLACK);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.drawBitmap(16, 9, image_device_power_button_bits, 30, 32, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
    }
    display.setCursor(7, 46);
    display.print("Power Off");

    if (powerMenuSelection == 1) {
      display.fillRoundRect(66, 4, 58, 56, 4, SH110X_WHITE);
      display.drawBitmap(80, 9, image_device_reset_bits, 30, 32, SH110X_BLACK);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.drawBitmap(80, 9, image_device_reset_bits, 30, 32, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
    }
    display.setCursor(78, 46);
    display.print("Reset");

    display.display();
  }
};

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
        pomodoroApp.run(display, upPressed, downPressed, selectPressed);
        break;
      case STATE_CALENDAR:
        calendarApp.run(display, network, audio, upPressed, downPressed, selectPressed);
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

URPocketSystem systemDevice;

void setup() {
  systemDevice.setup();
}

void loop() {
  systemDevice.loop();
}