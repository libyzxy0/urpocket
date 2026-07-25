#ifndef POMODOROAPP_H
#define POMODOROAPP_H

class PomodoroApp {
private:
  enum AppState { PAUSED, RUNNING };
  enum TimerMode { WORK_MODE, BREAK_MODE };

  struct Preset {
    int workSecs;
    int breakSecs;
    const char* label;
  };

  Preset presets[3] = {
    {25 * 60, 5 * 60,  "25/5"},  
    {50 * 60, 10 * 60, "50/10"},
    {15 * 60, 3 * 60,  "15/3"}
  };

  int currentPresetIndex = 0;
  AppState state = PAUSED;
  TimerMode mode = WORK_MODE;
  
  unsigned long lastTimerUpdate = 0;
  int secondsLeft = 25 * 60;
  int completedCycles = 0; 

  void resetTimerToCurrentPreset() {
    if (mode == WORK_MODE) {
      secondsLeft = presets[currentPresetIndex].workSecs;
    } else {
      secondsLeft = presets[currentPresetIndex].breakSecs;
    }
  }

public:
  void run(Adafruit_SH1106G &display, AudioSystem &audio, bool up, bool down, bool select) {
    if (select) {
      state = (state == RUNNING) ? PAUSED : RUNNING;
      audio.playClickSound();
    }

    if (state == PAUSED) {
      if (up) {
        currentPresetIndex = (currentPresetIndex + 1) % 3;
        resetTimerToCurrentPreset();
        audio.playClickSound();
      } else if (down) {
        currentPresetIndex = (currentPresetIndex - 1 + 3) % 3;
        resetTimerToCurrentPreset();
        audio.playClickSound();
      }
    }

    if (state == RUNNING) {
      if (millis() - lastTimerUpdate >= 1000) {
        lastTimerUpdate = millis();
        
        if (secondsLeft > 0) {
          secondsLeft--;
        } else {
          audio.playTimerEndAlarm();

          if (mode == WORK_MODE) {
            mode = BREAK_MODE;
            audio.playBreakStartSound();
          } else {
            mode = WORK_MODE;
            completedCycles++;
            audio.playWorkStartSound();
          }

          resetTimerToCurrentPreset();
          state = RUNNING; 
        }
      }
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    display.setCursor(0, 0);
    display.setTextSize(1);
    
    if (mode == WORK_MODE) {
      display.print("WORK ");
    } else {
      display.print("REST ");
    }

    display.print("#");
    display.print(completedCycles);
    display.print(" [");
    display.print(presets[currentPresetIndex].label);
    display.print("]");

    display.drawLine(0, 10, 128, 10, SH110X_WHITE);

    display.setCursor(18, 21);
    display.setTextSize(3);
    
    int mins = secondsLeft / 60;
    int secs = secondsLeft % 60;

    if (mins < 10) display.print("0");
    display.print(mins);
    display.print(":");
    if (secs < 10) display.print("0");
    display.print(secs);

    display.display();
  }
};

#endif // POMODOROAPP_H
