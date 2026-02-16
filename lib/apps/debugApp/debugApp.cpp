#include "debugApp.h"

static log_t Log;

void DebugApp::start() {
    statusTimer = new uint64_t;
    screen = new uint8_t;
    *statusTimer = 0;
    *screen = 0;
}

void DebugApp::update(char key, bool pressed, bool alt) {
    if(!pressed) return;
    switch(key) {
        // Handle key presses for the debug app
        case ' ':
            *screen = !*screen;
            break;
    }
}

void DebugApp::run() {
    // This function can be used to handle any additional logic needed in the debug app
    if(esp_timer_get_time() - *statusTimer > 250000) { // Update every 0.5 seconds
        *statusTimer = esp_timer_get_time(); // Clear the status bar area
        TFT_eSprite bar = TFT_eSprite(&tft);
        bar.createSprite(tft.width(), 15);
        bar.fillSprite(TFT_BLACK);
        renderStatusBar().pushToSprite(&bar, 0, 0, TFT_BLACK);
        frame.createSprite(tft.width(), tft.height());
        frame.loadFont(FONT_SMALL);
        frame.setTextColor(TFT_WHITE, TFT_BLACK);
        switch(*screen) {
            case 0:
                while(!taskHandler->takeSemaphore("status", portMAX_DELAY));
                while(!taskHandler->takeSemaphore("gps", portMAX_DELAY));
                frame.drawString("----------Debug App-----------\n", 0, 20);
                frame.println("Battery Voltage: " + String(Power::batteryVoltage) + "V");
                frame.println("Charge Rate: " + String(Power::chargeRate) + " % / hr");
                frame.println("GPS Status: " + String(GPS::gps.satellites.value()) + " satellites");
                frame.println("Latitude: " + String(GPS::gps.location.lat(), 6));
                frame.println("Longitude: " + String(GPS::gps.location.lng(), 6));
                frame.println("Speed: " + String(GPS::gps.speed.mph()) + " mph");
                frame.println("Free Memory: " + String(ESP.getFreeHeap()) + " / " + String(ESP.getHeapSize()) + " bytes");
                frame.println("Free PSRAM: " + String(ESP.getFreePsram()) + " / " + String(ESP.getPsramSize()) + " bytes");
                taskHandler->releaseSemaphore("status");
                taskHandler->releaseSemaphore("gps");
                bar.pushToSprite(&frame, 0, 0);
                frame.pushSprite(0, 0);
                break;
            case 1:
                frame.setCursor(0, 20);
                for(int i = 0; i < LOG_LINES; i++) {
                    if(i == Log.write - 1) frame.setTextColor(TFT_RED);
                    else frame.setTextColor(TFT_WHITE);
                    frame.print(Log.buffer[i]);
                }
                    
                bar.pushToSprite(&frame, 0, 0);
                frame.pushSprite(0, 0);
                break;
        }
        frame.deleteSprite();
        bar.deleteSprite();
    }
}

void DebugApp::end() {
    // Cleanup when exiting the debug app
    delete statusTimer; // Free the timer memory
    delete screen;
    statusTimer = nullptr; // Set to nullptr to avoid dangling pointer
    screen = nullptr;
}

void logWrite(const char* buf, size_t len) {
    //if(len > LOG_SIZE) return;
    snprintf(Log.buffer[Log.write], LOG_SIZE, buf);
    Log.write++;
    Log.write %= LOG_LINES;
}

int ramLog(const char* msg, va_list va) {
    char local_buf[128];
    int n = vsnprintf(local_buf, sizeof(local_buf), msg, va);
    if(n < 0) return n;
    if(n > sizeof(local_buf)) n = sizeof(local_buf);
    logWrite(local_buf, n);
    return vprintf(msg, va);
}

