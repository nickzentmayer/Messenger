#include "debugApp.h"

void DebugApp::start() {
    statusTimer = new uint64_t;
    *statusTimer = 0;
}

void DebugApp::update(char key, bool pressed, bool alt) {
    if(!pressed) return;
    switch(key) {
        // Handle key presses for the debug app
    }
}

void DebugApp::run() {
    // This function can be used to handle any additional logic needed in the debug app
    if(esp_timer_get_time() - *statusTimer > 1000000) { // Update every second
        *statusTimer = esp_timer_get_time(); // Clear the status bar area
        TFT_eSprite bar = TFT_eSprite(&tft);
        bar.createSprite(tft.width(), 15);
        bar.fillSprite(TFT_BLACK);
        while(!taskHandler->takeSemaphore("status", portMAX_DELAY));
        while(!taskHandler->takeSemaphore("gps", portMAX_DELAY));
        renderStatusBar().pushToSprite(&bar, 0, 0, TFT_BLACK);
        frame.createSprite(tft.width(), tft.height());
        frame.loadFont(FONT_SMALL);
        frame.setTextColor(TFT_WHITE, TFT_BLACK);
        frame.drawString("----------Debug App-----------\n", 0, 20);
        frame.println("Battery Voltage: " + String(batteryVoltage) + "V");
        frame.println("Charge Rate: " + String(chargeRate) + " % / hr");
        frame.println("GPS Status: " + String(GPS::gps.satellites.value()) + " satellites");
        frame.println("Latitude: " + String(GPS::gps.location.lat(), 6));
        frame.println("Longitude: " + String(GPS::gps.location.lng(), 6));
        frame.println("Altitude: " + String(GPS::gps.altitude.meters()) + " m");
        frame.println("Free Memory: " + String(ESP.getFreeHeap()) + " bytes");
        frame.println("Free PSRAM: " + String(ESP.getFreePsram()) + " bytes");
        taskHandler->releaseSemaphore("status");
        taskHandler->releaseSemaphore("gps");
        bar.pushToSprite(&frame, 0, 0);
        frame.pushSprite(0, 0);
        frame.deleteSprite();
        bar.deleteSprite();
    }
}

void DebugApp::end() {
    // Cleanup when exiting the debug app
    delete statusTimer; // Free the timer memory
    statusTimer = nullptr; // Set to nullptr to avoid dangling pointer
}
