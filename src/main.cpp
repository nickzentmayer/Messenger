#include <Arduino.h>

#include <Adafruit_TCA8418.h>
#include <TFT_eSPI.h>
#include "Adafruit_MAX1704X.h"
#include <RadioLib.h>
#include <WiFi.h>
#include <TinyGPS++.h>
#include <BleKeyboard.h>

Adafruit_MAX17048 maxlipo;
Adafruit_TCA8418 keypad;
TFT_eSPI tft = TFT_eSPI();
SX1262 lora = new Module(4, 16, 10, 17);
TinyGPSPlus gps;
BleKeyboard keyb;

#define ROWS 7
#define COLS 5

char keymap[COLS][ROWS] = {
    {'q', 'w', '@', 'a', '&', ' ', '+'},
    {'e', 's', 'd', 'p', 'x', 'z', '^'},
    {'r', 'g', 't', '^', 'v', 'c', 'f'},
    {'u', 'h', 'y', '\n', 'b', 'n', 'j'},
    {'o', 'l', 'i', '<', '$', 'm', 'k'}
};
char symKeymap[COLS][ROWS] = {
    {'#', '1', 'x', '*', '|', ' ', '0'},
    {'2', '4', '5', '@', '8', '7', '^'},
    {'3', '/', '(', '^', '?', '9', '6'},
    {'_', ':', ')', '\n', '!', ',', ';'},
    {'+', '"', '-', '<', '=', '.', '\''}
};
char capKeymap[COLS][ROWS] = {
    {'Q', 'W', '@', 'A', '&', ' ', '\\'},
    {'E', 'S', 'D', 'P', 'X', 'Z', '^'},
    {'R', 'G', 'T', '^', 'V', 'C', 'F'},
    {'U', 'H', 'Y', '\n', 'B', 'N', 'J'},
    {'O', 'L', 'I', '<', '$', 'M', 'K'}
};
bool sym = false;
bool shift = false;
bool lights = HIGH;
bool ap = false;
byte brightness = 150;
volatile bool receivedFlag = false;

ICACHE_RAM_ATTR
void setFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(0, INPUT);
  pinMode(LDO_EN, OUTPUT);
    digitalWrite(LDO_EN, HIGH);
  pinMode(KEY_BACKLIGHT, OUTPUT);
    digitalWrite(KEY_BACKLIGHT, HIGH);
    pinMode(TFT_BACKLIGHT, OUTPUT);
    //analogWrite(38, 100); 
  Wire.setPins(1, 2);
  if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    
    while (1)
      Serial.println("keypad not found, check wiring & pullups!");
  }
  if (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    //delay(2000);
  }
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("Hello World", 130, 75);
  delay(1000);
  Serial1.begin(9600);
  Serial1.setPins(44, 43);

  

  digitalWrite(21, LOW);

  // configure the size of the keypad matrix.
  // all other pins will be inputs
  keypad.matrix(ROWS, COLS);
  SPI.begin(11, 13, 12);
  lora.reset(true);
  tft.print(lora.begin(915.0));   
  lora.setDio2AsRfSwitch(true);
  lora.setBandwidth(500);
  lora.setSpreadingFactor(6);
  lora.setCodingRate(5);
  lora.setSyncWord(0x69);
  lora.setOutputPower(22);
  //lora.setRxBoostedGainMode(true);
  //lora.setCurrentLimit(100);
  lora.setTCXO(1.8);
  lora.setPacketReceivedAction(setFlag);

  // flush the internal buffer
  keypad.flush();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("Messenger", "ggez1234");
  WiFi.enableAP(ap);
    WiFi.disconnect();
    delay(100);
}

String message = "";
void loop() {
  while(Serial1.available()) {
    char c = Serial1.read();
    gps.encode(c);
    Serial.write(c);
  }
  if(receivedFlag) {
    receivedFlag = false;
    tft.println("Received packet: ");
    String tmp;
    tft.println(lora.readData(tmp));
    tft.print(tmp);
  }
  if (keypad.available() > 0) {
    digitalWrite(21, HIGH);
   // delay(100);
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
    Serial.print(" - ");
    Serial.print(keymap[col][row]);
    Serial.println();
    if(pressed) {
      if(keymap[col][row] == '<') {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 10);
        if(message.length() > 0) message = message.substring(0, message.length() - 1);
        tft.print(message);
      }
      else if(keymap[col][row] == '@') sym = !sym;
      else if(keymap[col][row] == '^') shift = !shift;
      else if(symKeymap[col][row] == '+' && sym) {
        brightness -= 5;
        analogWrite(38, brightness);
        analogWrite(21, map(brightness, 0, 255, 0, 60));
      }
      else if(symKeymap[col][row] == '-' && sym) {
        brightness += 5;
        analogWrite(38, brightness);
        analogWrite(21, map(brightness, 0, 255, 0, 60));
      }
      else if(keymap[col][row] == '+' && shift) {
        tft.println(String(maxlipo.cellPercent()) + " | " + String(maxlipo.cellVoltage()));
      }
      else if(keymap[col][row] == '+') {
        if(gps.time.isValid()) tft.println(String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + gps.time.second());
        tft.print(gps.location.lat(), 6);
        tft.print(", ");
        tft.print(gps.location.lng(), 6);
        tft.print(", ");
        tft.println(gps.satellites.value());
      }
      else if(symKeymap[col][row] == '=' && sym) {
        lora.startReceive();
      }
      else if(keymap[col][row] == '&') {
        int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        tft.println("no networks found");
    } else {
        tft.print(n);
        tft.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            tft.print(i + 1);
            tft.print(": ");
            tft.print(WiFi.SSID(i));
            tft.print(" (");
            tft.print(WiFi.RSSI(i));
            tft.print(")");
            tft.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
      }
      else if(keymap[col][row] == '\n') {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 10);
        int state = lora.transmit(message);
        tft.println(state);
        tft.println(lora.getDataRate());
        message = "";
      }
      else {
        if(sym) {
          tft.print(symKeymap[col][row]);
          message += symKeymap[col][row];
          //if(keyb.isConnected())keyb.print(symKeymap[col][row]);
        }
        else if(shift) {
          tft.print(capKeymap[col][row]);
          message += capKeymap[col][row];
          //if(keyb.isConnected())keyb.print(capKeymap[col][row]);
        }
        else {
          tft.print(keymap[col][row]);
          message += keymap[col][row];
          //if(keyb.isConnected())keyb.print(keymap[col][row]);
        }
      }
    }
  }
  if(!digitalRead(0)) {
    while(!digitalRead(0));
    if(lights) {
      analogWrite(38, brightness);
      analogWrite(21, map(brightness, 0, 255, 0, 60));
    }
    else {
      analogWrite(21, 255);
      analogWrite(38, 255);
    }
    lights = !lights;
  }
  //else analogWrite(21, 100);
}