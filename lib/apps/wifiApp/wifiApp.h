#pragma once
#ifndef WIFI_APP
#define WIFI_APP

#include "app.h"
#include <WiFi.h>

class WifiApp : public AppBase {
    public:
    WifiApp(TaskHandler* handler) : AppBase(handler) {}
    void start();
    void update(char key, bool pressed, bool alt);
    void run();
    void end();
    private:
    uint64_t* statusTimer; // Timer to manage status updates
};

#endif