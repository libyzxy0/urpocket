#ifndef SETTINGSAPP_H
#define SETTINGSAPP_H

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

#endif // SETTINGSAPP_H
