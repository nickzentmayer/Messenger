#pragma once
#ifndef POWER_H
#define POWER_H
#include <Arduino.h>
#include "TaskHandler.h"
#include <Adafruit_MAX1704X.h>
#include "LoRa.h"


class Power {
    public:
        Power() = delete; // Prevent instantiation
        static void devicesOn();
        static void devicesOff();
        static void changeBacklight(uint8_t brightness);
        static void beginSleep();
        static void pwrTask(void *pvParameters);
        static float batteryVoltage;
        static float batteryPercent;
        static float chargeRate;
    private:
        static TaskHandler *taskHandler;
};


#endif