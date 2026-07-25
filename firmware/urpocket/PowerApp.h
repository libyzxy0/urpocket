#ifndef POWERAPP_H
#define POWERAPP_H

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

#endif // POWERAPP_H
