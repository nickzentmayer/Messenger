#include <Arduino.h>
#include <TFT_eSPI.h>
#include <RadioLib.h>

// put function declarations here:
int myFunction(int, int);

TFT_eSPI tft(170, 240);

SX1262 lora = new Module(4, 17, 10, 14);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(115200);
  tft.begin();
  tft.fillScreen(0x000000);
  SPI.begin(11, 13, 12);
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  pinMode(GPIO_NUM_16, OUTPUT);
  digitalWrite(16, LOW);
  Serial.println(lora.begin(915.0));
  Serial1.begin(9600);
  Serial1.setPins(44, 43);
  lora.setDio2AsRfSwitch(true);
}

byte buff[255];

void loop() {
  // put your main code here, to run repeatedly:

  if(lora.receive(buff, 255) == 0) {
    for(int i = 0; i < 255; i++) Serial.write(buff[i]);
  }
  digitalWrite(16, LOW);
  //if(Serial1.available())Serial.write(Serial1.read());
 //delay(3000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
} 