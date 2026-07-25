#ifndef FLASHCARDSAPP_H
#define FLASHCARDSAPP_H

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

#endif // FLASHCARDSAPP_H
