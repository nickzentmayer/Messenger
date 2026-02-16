#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>
#include <HIDKeyboardTypes.h>


bool deviceConnected = false;
    BLEHIDDevice* hid;
    BLECharacteristic* input;
    BLECharacteristic* output;
    BLECharacteristic* inputReportChar;
    BLECharacteristic* outputReportChar;

class myCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      pServer->getAdvertising()->start();
    }
  };

  // HID Report Map describing a keyboard
const uint8_t reportMap[] = {
  0x05, 0x01,       // Usage Page (Generic Desktop)
  0x09, 0x06,       // Usage (Keyboard)
  0xA1, 0x01,       // Collection (Application)
  0x05, 0x07,       //   Usage Page (Key Codes)
  0x19, 0xE0,       //   Usage Minimum (224)
  0x29, 0xE7,       //   Usage Maximum (231)
  0x15, 0x00,       //   Logical Minimum (0)
  0x25, 0x01,       //   Logical Maximum (1)aaaaaaaaaaaaaa
  0x75, 0x01,       //   Report Size (1)
  0x95, 0x08,       //   Report Count (8)
  0x81, 0x02,       //   Input (Data, Variable, Absolute) ; Modifier byte
  0x95, 0x01,       //   Report Count (1)
  0x75, 0x08,       //   Report Size (8)
  0x81, 0x01,       //   Input (Constant) ; Reserved byte
  0x95, 0x06,       //   Report Count (6)
  0x75, 0x08,       //   Report Size (8)
  0x15, 0x00,       //   Logical Minimum (0)
  0x25, 0x65,       //   Logical Maximum (101)
  0x05, 0x07,       //   Usage Page (Key Codes)
  0x19, 0x00,       //   Usage Minimum (0)
  0x29, 0x65,       //   Usage Maximum (101)
  0x81, 0x00,       //   Input (Data, Array)
  0xC0              // End Collection
};
  

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  WiFi.mode(WIFI_STA);

  BLEDevice::init("ESP32S3 Keyboard");
  BLEServer* pServer = BLEDevice::createServer();
  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  pServer->setCallbacks(new myCallbacks());

  hid = new BLEHIDDevice(pServer);
  input = hid->inputReport(0); // Report ID
  output = hid->outputReport(0); // Report ID

  hid->manufacturer()->setValue("Nick Zentmayer");
  hid->pnp(0x02, 0x1234, 0x5678, 0x0110);
  hid->hidInfo(0x00, 0x01);
  hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));
  hid->startServices();

  hid->setBatteryLevel(100);

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->addServiceUUID(hid->deviceInfo()->getUUID());
  pAdvertising->start();
}

void loop(){
  if (deviceConnected) {
    // Example: Send a key press (e.g., 'a')
    uint8_t keyReport[8] = {0};
    keyReport[2] = 0x04; // HID usage ID for 'a'
    input->setValue(keyReport, sizeof(keyReport));
    input->notify();
    delay(100);
    // Release the key
    memset(keyReport, 0, sizeof(keyReport));
    input->setValue(keyReport, sizeof(keyReport));
    input->notify();
    delay(1000);
  }
  else {
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();  
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
  }
}