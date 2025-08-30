#pragma once
#ifndef POWER_H
#define POWER_H
#include <Arduino.h>
#include "TaskHandler.h"
#include <Adafruit_MAX1704X.h>
#include "LoRa.h"

extern float batteryVoltage;
extern float batteryPercent;
extern float chargeRate;

void devicesOn();
void devicesOff();
void changeBacklight(uint8_t brightness);
void beginSleep();

void pwrTask(void *pvParameters);


#endif