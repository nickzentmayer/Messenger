#pragma once
#ifndef DEBUG_H
#define DEBUG_H
#include <Arduino.h>
#include "app.h"
#include "power.h"
#include "GPS.h"

class DebugApp : public AppBase {
    public:
    DebugApp(TaskHandler* handler) : AppBase(handler) {}
    void start();
    void update(char key, bool pressed, bool alt);
    void run();
    void end();
    private:
    uint64_t* statusTimer; // Timer to manage status updates
    TFT_eSprite frame = TFT_eSprite(&tft);
};

#endif // DEBUG_H