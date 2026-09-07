#include <Arduino.h>

const int L1 = A0;
const int L2 = A1;
const int L3 = A2;

const int THRESHOLD = 10;
const unsigned long LOCK_MS = 100;   // минимален интервал между отчитания

int prevState = 0;         // 0 = никоя, 1 = L1, 2 = L2, 3 = L3
unsigned long lastTrigger = 0;

void setup() {
  Serial.begin(115200);
  analogReference(INTERNAL);
  ADCSRA = (ADCSRA & 0xF8) | 0x05;
}

void loop() {
  bool a1 = analogRead(L1) > THRESHOLD;
  bool a2 = analogRead(L2) > THRESHOLD;
  bool a3 = analogRead(L3) > THRESHOLD;

  // Текуща активна фаза (при застъпване — приоритет L1 > L2 > L3)
  int curState = 0;
  if (a1)      curState = 1;
  else if (a2) curState = 2;
  else if (a3) curState = 3;

  if (curState != prevState) {
    // Посока по реда на смяна на фазите
    bool cw  = (prevState==1 && curState==2) || (prevState==2 && curState==3) || (prevState==3 && curState==1);
    bool ccw = (prevState==1 && curState==3) || (prevState==3 && curState==2) || (prevState==2 && curState==1);

    if ((cw || ccw) && (millis() - lastTrigger >= LOCK_MS)) {
      if (cw)  Serial.println("Clockwise");
      else     Serial.println("Anti-clockwise");
      lastTrigger = millis();
    }

    prevState = curState;   // винаги следим състоянието, дори без печат
  }
}