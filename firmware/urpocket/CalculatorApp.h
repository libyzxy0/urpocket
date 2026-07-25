#ifndef CALCULATORAPP_H
#define CALCULATORAPP_H

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

#endif // CALCULATORAPP_H
