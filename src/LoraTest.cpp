// #include <Arduino.h>

// #include <Adafruit_TCA8418.h>
// #include <TFT_eSPI.h>
// #include "Adafruit_MAX1704X.h"
// #include <RadioLib.h>
// #include <TinyGPS++.h>

// Adafruit_MAX17048 maxlipo;
// Adafruit_TCA8418 keypad;
// TFT_eSPI tft = TFT_eSPI();
// SX1262 radio = new Module(4, 16, 10, 17);
// TinyGPSPlus gps;

// #define ROWS 7
// #define COLS 5
// //#define TX

// char keymap[COLS][ROWS] = {
//     {'*', '0', '#'},
//     {'7', '8', '9'},
//     {'4', '5', '6'},
//     {'1', '2', '3'},
// };

// volatile bool receivedFlag = false;

// // this function is called when a complete packet
// // is received by the module
// // IMPORTANT: this function MUST be 'void' type
// //            and MUST NOT have any arguments!
// #if defined(ESP8266) || defined(ESP32)
//   ICACHE_RAM_ATTR
// #endif
// void setFlag(void) {
//   // we got a packet, set the flag
//   receivedFlag = true;
// }

// void setup() {
//   //while(!Serial) delay(10);
//     Serial.begin(115200);
//   pinMode(14, OUTPUT);
//     digitalWrite(14, HIGH);
//   pinMode(21, OUTPUT);
//     digitalWrite(21, HIGH);
//     pinMode(38, OUTPUT);
//     digitalWrite(38, LOW); 
//   Wire.setPins(1, 2);
//   //if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    
//     //while (1)
//       Serial.println("keypad not found, check wiring & pullups!");
//  // }
//   /*while (!maxlipo.begin()) {
//     Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
//     delay(2000);
//   }*/

//   tft.begin();
//   tft.setRotation(1);
//   tft.fillScreen(TFT_WHITE);
//   tft.setTextColor(TFT_BLACK, TFT_WHITE);
//   tft.drawString("Hello World", 130, 75);
//   Serial1.begin(9600);
//   Serial1.setPins(44, 43);

//   //digitalWrite(21, LOW);

//   SPI.begin(11, 13, 12);
//   delay(2000);
//   radio.reset(true);
  
//   Serial.print(radio.begin(915.0));   
//   radio.setDio2AsRfSwitch(true);
//   radio.setBandwidth(7.8);
//   radio.setSpreadingFactor(12);
//   radio.setCodingRate(8);
//   radio.setOutputPower(22);
//   radio.setSyncWord(0x69);
//   radio.setRxBoostedGainMode(true);
//   //radio.setCurrentLimit(100);
//   radio.setTCXO(1.8);
//   #ifndef TX
//   radio.setPacketReceivedAction(setFlag);
//   radio.startReceive();
//   #endif
//   // configure the size of the keypad matrix
//   // all other pins will be inputs
//   keypad.matrix(ROWS, COLS);

//   // flush the internal buffer
//   keypad.flush();
// }
// int count = 0;
// int channelDetected = -1;
// void loop() {
//   #ifndef TX
// //   double offset = 0.0;
// //   while(channelDetected != -702) {
// //     radio.setFrequency(910.0 + offset);
// //     radio.scanChannel();
// //     channelDetected = radio.getChannelScanResult();
// //     Serial.print(F("[SX1262] Channel scan result: "));
// //     Serial.print(channelDetected);
// //     Serial.print(F(" ("));
// //     Serial.print(910.0 + offset);
// //     Serial.println(F(" MHz)"));
// //     offset += 0.05;
// //     if(offset > 10.0) offset = 0.0;
// //   }
//   if(receivedFlag) {
//     // reset flag
//     receivedFlag = false;
//     // radio.startReceive();

//     // you can read received data as an Arduino String
//     String str;
//     int state = radio.readData(str);

//     // you can also read received data as byte array
//     /*
//       byte byteArr[8];
//       int numBytes = radio.getPacketLength();
//       int state = radio.readData(byteArr, numBytes);
//     */

//     if (state == RADIOLIB_ERR_NONE) {
//       // packet was successfully received
//       Serial.println(F("[SX1278] Received packet!"));

//       // print data of the packet
//       Serial.print(F("[SX1278] Data:\t\t"));
//       Serial.println(str);

//       // print RSSI (Received Signal Strength Indicator)
//       Serial.print(F("[SX1278] RSSI:\t\t"));
//       Serial.print(radio.getRSSI());
//       Serial.println(F(" dBm"));

//       // print SNR (Signal-to-Noise Ratio)
//       Serial.print(F("[SX1278] SNR:\t\t"));
//       Serial.print(radio.getSNR());
//       Serial.println(F(" dB"));

//       // print frequency error
//       Serial.print(F("[SX1278] Frequency error:\t"));
//       Serial.print(radio.getFrequencyError());
//       Serial.println(F(" Hz"));

//     } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
//       // packet was received, but is malformed
//       Serial.println(F("[SX1278] CRC error!"));

//     } else {
//       // some other error occurred
//       Serial.print(F("[SX1278] Failed, code "));
//       Serial.println(state);

//     }
//   }
//   #else
// Serial.print(F("[SX1262] Transmitting packet ... "));

//   // you can transmit C-string or Arduino string up to
//   // 256 characters long
//   while(Serial1.available()) gps.encode(Serial1.read());
//   String str = "Lon: " + String(gps.location.lng());
//   str += "\nLat: " + String(gps.location.lat());
//   str += "\nSats: " + String(gps.satellites.value());
//   int state = radio.transmit(str);

//   // you can also transmit byte array up to 256 bytes long
//   /*
//     byte byteArr[] = {0x01, 0x23, 0x45, 0x56, 0x78, 0xAB, 0xCD, 0xEF};
//     int state = radio.transmit(byteArr, 8);
//   */

//   if (state == RADIOLIB_ERR_NONE) {
//     // the packet was successfully transmitted
//     Serial.println(F("success!"));

//     // print measured data rate
//     Serial.print(F("[SX1262] Datarate:\t"));
//     Serial.print(radio.getDataRate());
//     Serial.println(F(" bps"));

//   } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
//     // the supplied packet was longer than 256 bytes
//     Serial.println(F("too long!"));

//   } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
//     // timeout occured while transmitting packet
//     Serial.println(F("timeout!"));

//   } else {
//     // some other error occurred
//     Serial.print(F("failed, code "));
//     Serial.println(state);

//   }

//   // wait for a second before transmitting again
//   delay(1000);
//   #endif
// }