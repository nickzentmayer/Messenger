#include "GPS.h"

TinyGPSPlus GPS::gps;



void GPS::gpsTask(void *pvParameters) {
    // Your GPS task implementation here
    TaskHandler* taskHandler = (TaskHandler *)pvParameters;
    Serial1.begin(9600);
    Serial1.setPins(44, 43);

    log_d("GPS task started");
    for(;;) {
        while(!taskHandler->takeSemaphore("gps", portMAX_DELAY));
        while(Serial1.available()) {
    char c = Serial1.read();
    gps.encode(c);
        }
        taskHandler->releaseSemaphore("gps");
        if(gps.time.isUpdated()) {
            while(!taskHandler->takeSemaphore("status", portMAX_DELAY));
            //log_d("GPS Time: %02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
            struct tm t;
            t.tm_year = gps.date.year() - 1900; // Years since 1900
            t.tm_mon = gps.date.month() - 1;        // Months since January [0-11]
            t.tm_mday = gps.date.day();          // Day of the month [1-31]
            t.tm_hour = gps.time.hour();          // Hours since midnight [0-23]
            t.tm_min = gps.time.minute();           // Minutes after the hour [0-59]
            t.tm_sec = gps.time.second();            // Seconds after the minute [0-60]
            t.tm_isdst = 1;         // Daylight saving time flag
            time_t timeSinceEpoch = mktime(&t);

            struct timeval now = { .tv_sec = timeSinceEpoch + (TIMEZONE * 3600) + (DAYSAVETIME * 3600) };

            settimeofday(&now, NULL);
            

            taskHandler->releaseSemaphore("status");
        }
        vTaskDelay(500 / portTICK_PERIOD_MS); //Watchdog delay
    }
}
