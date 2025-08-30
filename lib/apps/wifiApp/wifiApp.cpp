#include "wifiApp.h"

void WifiApp::start() {
    // Initialize WiFi and other necessary components
    WiFi.mode(WIFI_STA);
    update(' ', true, false); // Call update to scan for networks
    statusTimer = new uint64_t;
    *statusTimer = 0;
    run();
}

void WifiApp::update(char key, bool pressed, bool alt) {
    if (!pressed) return; // Only process on key press
    if(key != ' ') return;
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(0, 20);
    int n = WiFi.scanNetworks();
    if (n == 0) {
        tft.println("no networks found");
    } else {
        tft.print(n);
        tft.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            tft.print(i + 1);
            tft.print(": ");
            tft.print(WiFi.SSID(i));
            tft.print(" (");
            tft.print(WiFi.RSSI(i));
            tft.print(")");
            tft.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
}
void WifiApp::run() {
    // This function can be used to handle any additional logic needed in the WiFi app
    // For now, it does nothing
    if(esp_timer_get_time() - *statusTimer > 1000000) { // Update every second
        *statusTimer = esp_timer_get_time();
        tft.fillRect(0, 0, tft.width(), 15, TFT_BLACK); // Clear the status bar area
        renderStatusBar().pushSprite(0, 0, TFT_BLACK);
    }
}
void WifiApp::end() {
    // Cleanup when exiting the WiFi app
    delete statusTimer; // Free the timer memory
    statusTimer = nullptr; // Set to nullptr to avoid dangling pointer
}