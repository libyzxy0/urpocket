#ifndef CHATAPP_H
#define CHATAPP_H

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

#endif // CHATAPP_H
