// #include <Arduino.h>
// #include <WiFi.h>
// #include <RadioLib.h>
// #include <WiFiClient.h>

// const char packetcode[3] = {4, 2, 0};

// SX1262 lora = new Module(4, 16, 10, 17);
// bool receivedFlag = false;
// String req;
// IRAM_ATTR
// void setFlag() {
//   // we got a packet, set the flag
//   String tmp;
//   if(lora.readData(tmp) == RADIOLIB_ERR_NONE) {
//     Serial.println("Received packet: " + tmp);
//     if(tmp.startsWith("req:")) {
//         req = tmp.substring(4);
//         receivedFlag = true;
//     }
// }
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
//   WiFi.begin("Verizon_6NSP4Q", "splash9-fax-con");
//   if(WiFi.waitForConnectResult(5000) != WL_CONNECTED) {
//     Serial.println("Failed to connect to WiFi");
//     while(1) delay(10000);
//   }
//   lora.setPacketReceivedAction(setFlag);
//   lora.startReceive();
// }

// void loop() {
//     if(receivedFlag) {
//         receivedFlag = false;
//         String host = req.substring(0, req.indexOf('/'));
//         String path = req.substring(req.indexOf('/'));
//         WiFiClient client;

//     if (!client.connect(host.c_str(), 80)) {
//         Serial.println("Connection failed.");
//         return;
//     }

//     // This will send a request to the server
//     //uncomment this line to send an arbitrary string to the server
//     //client.print("Send this data to the server");
//     //uncomment this line to send a basic document request to the server
//     client.print("GET " + path + " HTTP/1.1\n\n");
//     int maxloops = 0;
//     while (!client.available() && maxloops < 1000)
//   {
//     maxloops++;
//     delay(1); //delay 1 msec
//   }
//   if (client.available() > 0)
//   {
//     //read back one line from the server
//     String line = packetcode + client.readString() + packetcode;
//     while(line.length() > 255) {
//         String tmp = line.substring(0, 255);
//         lora.transmit(tmp);
//         line = tmp;
//     }
//         lora.transmit(line);
//   }
//   else
//   {
//     Serial.println("client.available() timed out ");
//   }

//     Serial.println("Closing connection.");
//     client.stop();
//         }
// }
