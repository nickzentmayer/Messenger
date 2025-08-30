#include <Arduino.h>

#include <Adafruit_TCA8418.h>
#include <TFT_eSPI.h>
#include "Adafruit_MAX1704X.h"
#include <RadioLib.h>
#include <WiFi.h>
#include <TinyGPS++.h>
#include <BleKeyboard.h>
#include "LittleFS.h"

Adafruit_MAX17048 maxlipo;
Adafruit_TCA8418 keypad;
TFT_eSPI tft = TFT_eSPI();
SX1262 lora = new Module(4, 16, 10, 17);
TinyGPSPlus gps;
BleKeyboard keyb;

#define ROWS 7
#define COLS 5

RTC_DATA_ATTR float freq = 915.0;
RTC_DATA_ATTR float bw = 125.0;
RTC_DATA_ATTR uint8_t cr = 8;
RTC_DATA_ATTR uint8_t sf = 12;


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
bool lights = true;
bool ap = false;
RTC_DATA_ATTR byte brightness = 150;
volatile bool receivedFlag = false;

ICACHE_RAM_ATTR
void setFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}

void setup() {
//   gpio_hold_dis(KEY_BACKLIGHT);
//   gpio_hold_dis(LDO_EN);
  Serial.begin(115200);
  pinMode(0, INPUT);
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
  int waitTime = 0;
  while(waitTime < 1000) {
    if(digitalRead(0)) {
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
      esp_deep_sleep_start();
    }
    delay(1);
    waitTime += 1;
  }
}

  pinMode(LDO_EN, OUTPUT);
    digitalWrite(LDO_EN, HIGH);
  pinMode(KEY_BACKLIGHT, OUTPUT);
    digitalWrite(KEY_BACKLIGHT, LOW);
    //digitalWrite(TFT_BACKLIGHT, LOW);
    //analogWrite(38, 100); 
  Wire.setPins(1, 2);
  if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    
    while (1)
      Serial.println("keypad not found, check wiring & pullups!");
  }
  if (!maxlipo.begin(&Wire, false)) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    //delay(2000);
  }
  tft.init();
  tft.setRotation(1);
  //tft.loadFont(FONT_SMALL);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Hello World", 130, 75);
  delay(100);
  Serial1.begin(9600);
  Serial1.setPins(44, 43);
  pinMode(TFT_BACKLIGHT, OUTPUT);

  

  analogWrite(TFT_BACKLIGHT, brightness);
  analogWrite(KEY_BACKLIGHT, map(brightness, 0, 255, 0, 60));

  // configure the size of the keypad matrix.
  // all other pins will be inputs
  keypad.matrix(ROWS, COLS);
  SPI.begin(11, 13, 12);
  lora.reset(true);
  tft.print(lora.begin(freq));   
  lora.setDio2AsRfSwitch(true);
  lora.setBandwidth(bw);
  lora.setSpreadingFactor(sf);
  lora.setCodingRate(cr);
  //lora.setSyncWord(0x69);
  lora.setOutputPower(22);
  lora.setRxBoostedGainMode(true);
  //lora.setCurrentLimit(140);
  lora.setTCXO(3.3);
  lora.setPacketReceivedAction(setFlag);

  // flush the internal buffer
  keypad.flush();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("Messenger", "ggez1234");
  WiFi.enableAP(ap);
    WiFi.disconnect();
    delay(100);
    lora.startReceive();
    while(!digitalRead(0)) delay(1);
}
bool justTx = false;
String message = "";
void loop() {
  while(Serial1.available()) {
    char c = Serial1.read();
    gps.encode(c);
    Serial.write(c);
  }
  if(receivedFlag) {
    receivedFlag = false;
    String tmp;
    int state = lora.readData(tmp);
    if(justTx) {
        justTx = false;
        return;
    }
    tft.fillScreen(TFT_WHITE);
    tft.setCursor(10, 10);
    tft.println("Received packet: ");
    tft.println(state);
    tft.println(tmp + "\n");
    tft.print("Recived at: " + String(lora.getRSSI()) + " dBm");
  }
  if (keypad.available() > 0) {
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
        tft.fillScreen(TFT_WHITE);
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
      else if(keymap[col][row] == '+' && shift && !sym) {
        tft.println(String(maxlipo.cellPercent()) + " | " + String(maxlipo.cellVoltage()) + " | " + String(maxlipo.chargeRate()));
      }
      else if(keymap[col][row] == '+' && !sym) {
        if(gps.time.isValid()) tft.println(String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + gps.time.second());
        tft.print(gps.location.lat(), 7);
        tft.print(", ");
        tft.print(gps.location.lng(), 7);
        tft.print(", ");
        tft.print(gps.speed.mph(), 3);
        tft.print(", ");
        tft.println(gps.satellites.value());
        
;      }
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
        tft.fillScreen(TFT_WHITE);
        tft.setCursor(10, 10);
        justTx = true;
        int state = lora.transmit(message);
        tft.println(state);
        tft.println(lora.getDataRate());
        if(message.startsWith("/")) {
          message = message.substring(1);
          if(message.startsWith("f ")) {
            freq = message.substring(2).toFloat();
            lora.setFrequency(message.substring(2).toFloat());
            tft.println("Frequency set to: " + message.substring(2));
        }
          else if(message.startsWith("p ")) {
            lora.setOutputPower(message.substring(2).toInt());
            tft.println("Output power set to: " + message.substring(2));
          }
          else if(message.startsWith("b ")) {
            bw = message.substring(2).toFloat();
            lora.setBandwidth(message.substring(2).toFloat());
            tft.println("Bandwidth set to: " + message.substring(2));
          }
          else if(message.startsWith("sf ")) {
            sf = message.substring(3).toInt();
            lora.setSpreadingFactor(message.substring(3).toInt());
            tft.println("Spreading factor set to: " + message.substring(3));
          }
          else if(message.startsWith("cr ")) {
            cr = message.substring(3).toInt();
            lora.setCodingRate(message.substring(3).toInt());
            tft.println("Coding rate set to: " + message.substring(3));
          }
        }
        
        lora.startReceive();
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
    if(!lights) {
      analogWrite(38, brightness);
      analogWrite(21, map(brightness, 0, 255, 0, 60));
    }
    else {
      analogWrite(21, 255);
      analogWrite(38, 255);
      digitalWrite(LDO_EN, HIGH);
      lora.sleep();
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
    //   pinMode(KEY_BACKLIGHT, OUTPUT);
    //   digitalWrite(KEY_BACKLIGHT, LOW);
    //   gpio_hold_en(KEY_BACKLIGHT);
    //   gpio_hold_en(LDO_EN);
    //   gpio_deep_sleep_hold_en();
      delay(100);
      esp_deep_sleep_start();
    }
    lights = !lights;
  }
  //else analogWrite(21, 100);
}