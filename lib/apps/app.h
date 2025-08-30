#pragma once
#ifndef APP_H
#define APP_H

#include <TFT_eSPI.h>
#include "guiFunctions.h"
#include "TaskHandler.h"

#define APP_COUNT 3 // Number of apps in the system, home does not count

extern TFT_eSprite bkg;

class AppBase {
public:
    AppBase(TaskHandler* handler) : taskHandler(handler) {}
    virtual void start() = 0;
    virtual void run() = 0;
    virtual void update(char, bool, bool) = 0;
    virtual void end() = 0;
protected:
    TaskHandler* taskHandler; // Task handler for managing semaphores
    int appState; // Current state of the app
};

#endif