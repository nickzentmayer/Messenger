#ifndef HID_KEYBOARD_H
#define HID_KEYBOARD_H

#include <Arduino.h>
#include "app.h"
#include <BLEDevice.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>
#include <HIDKeyboardTypes.h>
#include "power.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "nvs_flash.h"
#include "esp_gap_ble_api.h"


struct InputReport {
    uint8_t modifiers;	     // bitmask: CTRL = 1, SHIFT = 2, ALT = 4
    uint8_t reserved;        // must be 0
    uint8_t pressedKeys[6];  // up to six concurrently pressed keys
};

class HIDKeyboard : public AppBase {
    public:
    HIDKeyboard(TaskHandler* handler) : AppBase(handler) {}
    void start();
    void update(char key, bool pressed, bool alt);
    void run();
    void end();
    private:
    uint64_t* statusTimer; // Timer to manage status updates
    uint8_t* appState;
    bool* selection;
    TFT_eSprite* frame;
    BLEHIDDevice* hid;
    BLEServer* pServer;
    BLESecurity* pSecurity;
    BLECharacteristic* input;
    BLECharacteristic* output;
    BLECharacteristic* bootIn;
    BLECharacteristic* bootOut;
    //USBHIDKeyboard* USBKeyboard;
    void setupBLE();
    void setupUSB();
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
#endif