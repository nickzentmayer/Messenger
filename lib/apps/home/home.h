#pragma once
#ifndef HOME_H
#define HOME_H

#include <Arduino.h>
#include "app.h"

extern int currentApp;

struct Icon{
    String name;
    String location;
    TFT_eSprite sprite = TFT_eSprite(&tft);
    Icon(String n, String l) : name(n), location(l) {}
};

class Home : public AppBase {
    public:
    Home(TaskHandler* handler) : AppBase(handler) {}
    void start();
    void update(char key, bool pressed, bool alt);
    void run();
    void end();
private:
    void animateIcons(bool direction);
    void loadIcons();
    void unloadIcons();
    TFT_eSprite frame = TFT_eSprite(&tft);
    Icon icons[APP_COUNT] = {{"Messages", "/assets/messages.png"},
                 {"WiFi", "/assets/wifi-icon.png"},
                 {"Debug", "/assets/debug.png"}}; 
    uint64_t* statusTimer; // Timer to manage status updates

};

#endif