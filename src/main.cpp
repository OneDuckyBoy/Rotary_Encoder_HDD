#include <Arduino.h>
#include <HID-Project.h>   // Consumer (звук) + Mouse (скрол)

// Фази на мотора
const int L1 = A0;
const int L2 = A1;
const int L3 = A2;
const int BTN = 8;         // бутон за смяна на режима (чете се тук)
const int BTN_GND = 6;     // виртуален GND (държим на LOW)
const int LED_PIN = 4;     // LED индикация за VOLUME режим

const int THRESHOLD = 10;
const unsigned long LOCK_MS = 40;   // колко бързо реагира на въртене

int prevState = 0;
unsigned long lastTrigger = 0;

bool volumeMode = true;      // true = звук, false = скрол
bool lastBtn = HIGH;
unsigned long lastBtnTime = 0;

void setup() {
  Serial.begin(115200);
  analogReference(INTERNAL);
  ADCSRA = (ADCSRA & 0xF8) | 0x05;

  pinMode(BTN_GND, OUTPUT);
  digitalWrite(BTN_GND, LOW);   // действа като GND за бутона
  pinMode(BTN, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, volumeMode ? HIGH : LOW);  // свети само в VOLUME режим

  Consumer.begin();   // медиа клавиши
  Mouse.begin();      // мишка (колело)
}

void loop() {
  // --- Бутон: toggle режим (falling edge + debounce) ---
  bool btn = digitalRead(BTN);
  if (btn == LOW && lastBtn == HIGH && (millis() - lastBtnTime > 50)) {
    volumeMode = !volumeMode;
    digitalWrite(LED_PIN, volumeMode ? HIGH : LOW);  // актуализирай LED
    Serial.print("Mode: ");
    Serial.println(volumeMode ? "VOLUME" : "SCROLL");
  }
  lastBtn = btn;

  // --- Rotary: чети фазите и определяй посока ---
  bool a1 = analogRead(L1) > THRESHOLD;
  bool a2 = analogRead(L2) > THRESHOLD;
  bool a3 = analogRead(L3) > THRESHOLD;

  int curState = 0;
  if (a1)      curState = 1;
  else if (a2) curState = 2;
  else if (a3) curState = 3;

  if (curState != prevState) {
    bool cw  = (prevState==1 && curState==2) || (prevState==2 && curState==3) || (prevState==3 && curState==1);
    bool ccw = (prevState==1 && curState==3) || (prevState==3 && curState==2) || (prevState==2 && curState==1);

    if ((cw || ccw) && (millis() - lastTrigger >= LOCK_MS)) {
      if (volumeMode) {
        if (cw)  Consumer.write(MEDIA_VOLUME_UP);
        else     Consumer.write(MEDIA_VOLUME_DOWN);
      } else {
        if (cw)  Mouse.move(0, 0, 1);    // скрол
        else     Mouse.move(0, 0, -1);
      }
      lastTrigger = millis();
    }

    prevState = curState;
  }
}