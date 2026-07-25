#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

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
  void playWorkStartSound() {
    tone(pin, 880, 100); delay(120);
    tone(pin, 1760, 150); delay(150);
  }

  void playBreakStartSound() {
    tone(pin, 1568, 100); delay(120);
    tone(pin, 1175, 100); delay(120);
    tone(pin, 784, 200);  delay(200);
  }

  void playTimerEndAlarm() {
    for (int i = 0; i < 3; i++) {
      tone(pin, 2000, 80); delay(100);
      tone(pin, 2000, 80); delay(150);
    }
  }
};

#endif // AUDIOSYSTEM_H
