#include <Arduino.h>

#include <Adafruit_TCA8418.h>
#include <TFT_eSPI.h>
#include "Adafruit_MAX1704X.h"

Adafruit_MAX17048 maxlipo;
Adafruit_TCA8418 keypad;
TFT_eSPI tft = TFT_eSPI();

#define ROWS 7
#define COLS 5

char keymap[COLS][ROWS] = {
    {'*', '0', '#'},
    {'7', '8', '9'},
    {'4', '5', '6'},
    {'1', '2', '3'},
};

void setup() {
  Serial.begin(115200);
  pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);
  pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    pinMode(38, OUTPUT);
    digitalWrite(38, LOW); 
  Wire.setPins(1, 2);
  if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    
    while (1)
      Serial.println("keypad not found, check wiring & pullups!");
  }
  while (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    delay(2000);
  }

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Hello World", 130, 75);
  Serial1.begin(9600);
  Serial1.setPins(44, 43);

  digitalWrite(21, LOW);

  // configure the size of the keypad matrix.
  // all other pins will be inputs
  keypad.matrix(ROWS, COLS);

  // flush the internal buffer
  keypad.flush();
}

void loop() {
  while(Serial1.available()) Serial.write(Serial1.read());
  if (keypad.available() > 0) {
    digitalWrite(21, HIGH);
    delay(100);
    //  datasheet page 15 - Table 1
    int k = keypad.getEvent();
    bool pressed = k & 0x80;
    k &= 0x7F;
    k--;
    uint8_t row = k / 10;
    uint8_t col = k % 10;

    if (pressed)
      Serial.print("PRESS\tR: ");
    else
      Serial.print("RELEASE\tR: ");
    Serial.print(row);
    Serial.print("\tC: ");
    Serial.print(col);
 //   Serial.print(" - ");
 //   Serial.print(keymap[col][row]);
    Serial.println();
  }
  else digitalWrite(21, LOW);
}
