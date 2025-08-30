// #include <Arduino.h>
// #include <WiFi.h>
// #include <RadioLib.h>
// #include <DNSServer.h>
// #include <WebServer.h>

// const char packetcode[3] = {4, 2, 0};

// SX1262 lora = new Module(4, 16, 10, 17);
// WebServer server(80);
// DNSServer dnsServer;
// const char* ssid = "Messenger";
// const char* password = "ggez123456";
// bool receivedFlag = false;
// String data;
// bool aGO = false;

// IRAM_ATTR
// void setFlag() {
//     String tmp;
//   if(lora.readData(tmp) == RADIOLIB_ERR_NONE) {
//     Serial.println("Received packet: " + tmp);
//     if(tmp.startsWith(packetcode)) {
//         aGO = true;
//         tmp = tmp.substring(3);
//     }
//     if(aGO) {
//         if(tmp.endsWith(packetcode)) {
//             data += tmp.substring(0, tmp.length() - 3);
//             aGO = false;
//             receivedFlag = true;
//         }
//         else {
//             data += tmp;
//         }
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   lora.setFrequency(915.0);
//   lora.setSpreadingFactor(7);
//   lora.setBandwidth(125);
//   lora.setCodingRate(5);
//   lora.setPreambleLength(8);
//   lora.setSyncWord(0x12);
//   lora.setOutputPower(22);
//   lora.setPacketReceivedAction(setFlag);
//   WiFi.softAP(ssid, password);
//   dnsServer.start(53, "*", WiFi.softAPIP());
//     server.on("/", []() {
//         String req = "req:" + server.uri();
//         lora.transmit(req);
//         lora.startReceive();
//         uint64_t start = millis();
//         while (!receivedFlag && millis() - start < 5000 && !aGO) {
//             delay(1);
//         }
//         if(!receivedFlag) {
//             server.send(200, "text/plain", "Timeout");
//             return;
//         }
//         else server.send(200, "text/html", data);
//         data = "";
//         receivedFlag = false;
//     });
// }

// void loop() {
//   dnsServer.processNextRequest();
//   server.handleClient();
// }