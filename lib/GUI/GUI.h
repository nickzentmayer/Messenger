#pragma once
#ifndef GUI_H
#define GUI_H
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "TaskHandler.h"
#include "Keyboard.h"
#include "guiFunctions.h"
#include "home/home.h"
#include "wifiApp/wifiApp.h"
#include "debugApp/debugApp.h"
#include "messages/messages.h"

#define FONT_SMALL NotoSansBold15
#define FONT_LARGE NotoSansBold36

enum AppType {
    HOME,
    MESSAGES,
    WIFI,
    DEBUG_APP
};

void guiTask(void *pvParameters);


#endif