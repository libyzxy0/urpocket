#ifndef ALERTSAPP_H
#define ALERTSAPP_H

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

#endif // ALERTSAPP_H
