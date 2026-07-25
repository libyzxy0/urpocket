#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H

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

#endif // INPUTCONTROLLER_H
