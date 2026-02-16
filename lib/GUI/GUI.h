#pragma once
#ifndef GUI_H
#define GUI_H
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "TaskHandler.h"
#include "Keyboard.h"
#include "guiFunctions.h"
#include "home/home.h"
#include "wifiApp/wifiApp.h"
#include "hidKeyboard/hidKeyboard.h"
#include "debugApp/debugApp.h"
#include "messages/messages.h"

#define FONT_SMALL NotoSansBold15
#define FONT_LARGE NotoSansBold36

enum AppType {
    HOME,
    MESSAGES,
    WIFI,
    DEBUG_APP,
    KEYBOARD
};
class GUI {
    public:
        static void guiTask(void *pvParameters);
    private:
        GUI() = delete; // Prevent instantiation

        static void processKey(char key, bool pressed);
        static void processLoop();

        static AppBase* apps[APP_COUNT + 1];
        static bool shift;
        static bool caps;
        static bool syms;
        static bool alt;
};



#endif