#pragma once
#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "TaskHandler.h"
#include <time.h>

#define TIMEZONE -5 // Set your timezone offset in hours
#define DAYSAVETIME 1 // Set to 1 if Daylight Saving Time is observed,

 
class GPS {
    public:
    GPS() = delete; // Prevent instantiation
    static void gpsTask(void *pvParameters);
    static TinyGPSPlus gps;
};

#endif // GPS_H