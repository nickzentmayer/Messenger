#pragma once
#ifndef DEBUG_H
#define DEBUG_H
#include <Arduino.h>
#include "app.h"
#include "power.h"
#include "GPS.h"

#define LOG_SIZE 100
#define LOG_LINES 9

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
    uint8_t* screen;
    char* tempLog[7];
};

typedef struct {
    volatile uint16_t write;
    volatile uint16_t read;
    char buffer[LOG_LINES][LOG_SIZE];
} log_t;

int ramLog(const char* msg, va_list va);

#endif // DEBUG_H