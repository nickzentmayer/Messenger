#include "hidKeyboard.h"

bool deviceConnected = false;
bool capsLock = false;

USBHIDKeyboard USBKeyboard;


class myCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      pServer->getServiceByUUID("1812")->getCharacteristic(BLEUUID((uint16_t)0x2A4D))->setNotifyProperty(true); // Set the protocol mode to "report"
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        InputReport report = {
                .modifiers = 0,
                .reserved = 0,
                .pressedKeys = {
                    0,
                    0, 0, 0, 0, 0
                }
            };
        pServer->getServiceByUUID("1812")->getCharacteristic(BLEUUID((uint16_t)0x2A4D))->setValue((uint8_t*)&report, sizeof(report)); // Clear report
        pServer->getAdvertising()->start();
    }
    void onNotify(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
        char* data = (char*)param->write.value;
        log_e("Output report received: %02x", data[0]);
    }
  };

 class OutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) {
        //todo: handle output reports from the host
    }
};

void HIDKeyboard::start() {
    statusTimer = new uint64_t;
    appState = new uint8_t;
    *appState = 0;
    selection = new bool;
    *selection = false;
    frame = new TFT_eSprite(&tft);
    frame->createSprite(tft.width(), tft.height()); // Create a sprite for the status
    frame->loadFont(FONT_SMALL);
    frame->setTextColor(TFT_WHITE, TFT_BLACK);
    frame->fillSprite(TFT_BLACK);
    int x;
    centerText("Select Keyboard Mode", &x, nullptr);
    frame->drawString("Select Keyboard Mode", x, 20);
    frame->setTextColor(((*selection) ? TFT_WHITE : TFT_RED), TFT_BLACK);
    frame->drawString("USB", 60, 120);
    frame->setTextColor((!(*selection) ? TFT_WHITE : TFT_RED), TFT_BLACK);
    frame->drawString("BLE", 180, 120);
    frame->pushSprite(0, 0);
    *statusTimer = 0;
    
}

void HIDKeyboard::update(char key, bool pressed, bool alt) {
    if(*appState == 0) {
        if(key == '\n' && pressed) {
            *appState = *selection + 1;
            switch(*appState) {
                case 1:
                    log_e("USB Keyboard selected - setting up USB");
                    setupUSB();
                    break;
                case 2:
                    log_e("BLE Keyboard selected - setting up BLE");
                    setupBLE();
                    break;
            }
            return;
        }
        if(key == 'a' && pressed) {
            *selection = false;
        }
        else if(key == 'd' && pressed) {
            *selection = true;
        }
        frame->fillSprite(TFT_BLACK);
        int x;
        centerText("Select Keyboard Mode", &x, nullptr);
        frame->drawString("Select Keyboard Mode", x, 20);
        frame->setTextColor((!(*selection) ? TFT_RED : TFT_WHITE), TFT_BLACK);
        frame->drawString("USB", 60, 120);
        frame->setTextColor(((*selection) ? TFT_RED : TFT_WHITE), TFT_BLACK);
        frame->drawString("BLE", 240, 120);
        renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
        frame->pushSprite(0, 0);
        frame->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    
    else if(*appState == 1) {
        //if(!USBKeyboard->availableForWrite()) USBKeyboard->println("hi"); // If not ready, do nothing
        if(key == '&' || key == '^') return; // Ignore modifier keys
        if(alt) {
            switch(key) {
                        case 'i':
                            key = KEY_UP_ARROW;
                            break;
                        case 'j':
                            key = KEY_LEFT_ARROW;
                            break;
                        case 'm':
                            key = KEY_DOWN_ARROW;
                            break;
                        case 'l':
                            key = KEY_RIGHT_ARROW;
                            break;
                        case 'e':
                            key = KEY_PAGE_UP;
                            break;
                        case 'q':
                            key = KEY_PAGE_DOWN;
                            break;
                        case 'r':
                            key = KEY_HOME;
                            break;
                        case 'f':
                            key = KEY_END;
                            break;
                        case 't':
                            key = KEY_TAB;
                            break;
                    }
        }
        if(pressed) {
            USBKeyboard.press(key);
        } else {
            USBKeyboard.release(key);
        }
    }
    else if(*appState == 2) {
        if(!deviceConnected) return; // If not connected, do nothing
        if(key == '&' || key == '^') return; // Ignore modifier keys
        if(pressed) {
            KEYMAP map;
            if (key > KEYMAP_SIZE)
                    return; // character not available on keyboard - skip
                if(!alt) {
                    map = keymap[key];
                }
                else {
                    map = keymap[KEY_CTRL];
                    switch(key) {
                        case 'i':
                            map = keymap[UP_ARROW];
                            break;
                        case 'j':
                            map = keymap[LEFT_ARROW];
                            break;
                        case 'm':
                            map = keymap[DOWN_ARROW];
                            break;
                        case 'l':
                            map = keymap[RIGHT_ARROW];
                            break;
                        case 'e':
                            map = keymap[KEY_PAGE_UP];
                            break;
                        case 'q':
                            map = keymap[KEY_PAGE_DOWN];
                            break;
                        case 'r':
                            map = keymap[KEY_HOME];
                            break;
                    }
                }
                

                // create input report
                InputReport report = {
                    .modifiers = map.modifier,
                    .reserved = 0,
                    .pressedKeys = {
                        map.usage,
                        0, 0, 0, 0, 0
                    }
                };
                input->setValue((uint8_t*)&report, sizeof(report));
                input->notify();
            } else {
                // send empty report
                InputReport report = {0, 0, {0, 0, 0, 0, 0, 0}};
                input->setValue((uint8_t*)&report, sizeof(report));
                input->notify();
            }
    }
}

void HIDKeyboard::run() {
    // This function can be used to handle any additional logic needed in the HIDKeyboard app
    if(esp_timer_get_time() - *statusTimer > 1000000) { // Update every second
    *statusTimer = esp_timer_get_time(); // Clear the status bar area
        if(*appState == 2) {
                if(!deviceConnected) {
                frame->fillSprite(TFT_BLACK);
                int x, y;
                centerText("Disconnected", &x, &y);
                frame->drawString("Disconnected", x, y);
                frame->pushSprite(0, 0);
            } else {
                // If connected, we can show a "connected" status
                frame->fillSprite(TFT_BLACK);
                frame->drawString(String((const char*)output->getData()), 0, 0);
                int x, y;
                centerText("Connected", &x, &y);
                frame->drawString("Connected", x, y);
                frame->pushSprite(0, 0);
                
            }
        }
    }
}

void HIDKeyboard::end() {
    // Cleanup when exiting the HIDKeyboard app
    delete statusTimer; // Free the timer memory
    log_e("Deinitializing");
    frame->deleteSprite(); // Delete the sprite
    delete frame;
    deviceConnected = false;
    if(*appState == 1) {
        USBKeyboard.end();
    }
    else if(*appState == 2) {
        //delete hid; --- IGNORE ---
        pServer->getAdvertising()->stop();
        pServer->disconnect(pServer->getConnId());
        BLEDevice::deinit(false);
        delete pSecurity;
        delete input;
        delete output;
        delete bootIn;
        delete bootOut;
    }
    delete appState;
    delete selection;
    // hello from messenger!!!&
}

void HIDKeyboard::setupBLE() {
  BLEDevice::init("Messenger");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new myCallbacks());
  pSecurity = new BLESecurity();
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  
    //wow it rlly works!!&&&&
  log_e("Setting ups BLE");
  //this is so cool
  hid = new BLEHIDDevice(pServer);
  input = hid->inputReport(0); // Report ID
  output = hid->outputReport(0); // Report ID
   bootIn = hid->bootInput();
   bootOut = hid->bootOutput();

//    InputReport report = {0, 0, {0, 0, 0, 0, 0, 0}};
//    bootIn->setValue((uint8_t*)&report, sizeof(report));

  hid->manufacturer()->setValue("Nick Zentmayer");
  hid->pnp(0x02, 0x1234, 0x5678, 0x0110);
  hid->hidInfo(0x00, 0x01);
  hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));
  hid->startServices();

  hid->setBatteryLevel(Power::batteryPercent);

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->addServiceUUID(hid->deviceInfo()->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  
}

void HIDKeyboard::setupUSB() {
    //USBKeyboard = new USBHIDKeyboard;
    USBKeyboard.begin();
    USBKeyboard.clearWriteError();
    USB.begin();
    frame->fillSprite(TFT_BLACK);
    int x, y;
    centerText("USB Keyboard Ready", &x, &y);
    frame->drawString("USB Keyboard Ready", x, y);
    frame->pushSprite(0, 0);
}