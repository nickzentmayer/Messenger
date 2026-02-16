#pragma once
#ifndef WIFI_APP
#define WIFI_APP

#include "app.h"
#include <WiFi.h>
#include <Preferences.h>

#define CRED_USERNAME 1
#define CRED_PASSWORD 0

enum WifiState {
    SCANNING,
    PASSWORD,
    USERNAME
};

class WifiApp : public AppBase {
    public:
    WifiApp(TaskHandler* handler) : AppBase(handler) {}
    void start();
    void update(char key, bool pressed, bool alt);
    void run();
    void end();
    private:
    void drawBars(uint16_t y, int32_t rssi); // Function to draw signal strength bars
    void connect(wifi_auth_mode_t auth, String ssid);
    uint64_t* statusTimer; // Timer to manage status updates
    uint32_t* selection; // Current selection index
    int16_t* numNetworks; // Number of networks found
    uint8_t* appState; // Current state of the app
    String* credentials;
    TFT_eSprite* frame;
};

#endif