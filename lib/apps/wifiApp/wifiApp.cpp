#include "wifiApp.h"

void WifiApp::start() {
    // Initialize WiFi and other necessary components
    frame = new TFT_eSprite(&tft);
    frame->createSprite(tft.width(), tft.height());
    frame->fillScreen(TFT_BLACK);
    frame->loadFont(FONT_SMALL);
    frame->setTextColor(TFT_WHITE, TFT_BLACK);
    int x, y;
    centerText("Loading...", &x, &y);
    frame->drawString("Loading...", x, y);
    frame->pushSprite(0, 0);
    statusTimer = (uint64_t*) ps_malloc(sizeof(uint64_t));
    selection = (uint32_t*) ps_malloc(sizeof(uint32_t));
    numNetworks = (int16_t*) ps_malloc(sizeof(int16_t));
    appState = (uint8_t*) ps_malloc(sizeof(uint8_t));
    credentials = new String[2](); // Allocate space for username and password
    *numNetworks = 0;
    *statusTimer = 0;
    *selection = 0;
    *appState = 0;
    while(!taskHandler->takeSemaphore("WiFi", portMAX_DELAY));
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    update(' ', true, false); // Call update to scan for networks
}

void WifiApp::update(char key, bool pressed, bool alt) {
    if (!pressed) return; // Only process on key press
    if(key == '\n') {
        if(*numNetworks == 0) return; // No networks to select
        switch(*appState) {
            case SCANNING:
                // Connect to the selected network
                switch(WiFi.encryptionType(*selection)) {
                    case WIFI_AUTH_OPEN:
                        connect(WIFI_AUTH_OPEN, WiFi.SSID(*selection).c_str());
                        break;
                case WIFI_AUTH_WEP:
                case WIFI_AUTH_WPA_PSK:
                case WIFI_AUTH_WPA2_PSK:
                case WIFI_AUTH_WPA_WPA2_PSK:
                case WIFI_AUTH_WPA3_PSK:
                case WIFI_AUTH_WPA2_WPA3_PSK:
                    *appState = PASSWORD; // Move to password entry state
                    frame->fillSprite(TFT_BLACK);
                    frame->setCursor(0, 20);
                    frame->setTextColor(TFT_WHITE, TFT_BLACK);
                    frame->println("Enter password for " + WiFi.SSID(*selection) + ":\n");
                    renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
                    frame->pushSprite(0, 0);
                    break;
                //case WIFI_AUTH_ENTERPRISE:
                case WIFI_AUTH_WPA2_ENTERPRISE:
                    *appState = USERNAME; // Move to enterprise credentials entry state
                    frame->fillSprite(TFT_BLACK);
                    frame->setCursor(0, 20);
                    frame->setTextColor(TFT_WHITE, TFT_BLACK);
                    frame->println("Enter username for " + WiFi.SSID(*selection) + ":\n");
                    renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
                    frame->pushSprite(0, 0);
                    break;
                default:
                    frame->fillSprite(TFT_BLACK);
                    int x, y;
                    centerText("Unsupported encryption type", &x, &y);
                    frame->drawString("Unsupported encryption type", x, y);
                    frame->pushSprite(0, 0);
                    delay(1000);
                    *appState = SCANNING; // Reset state to scanning
                    
            }
            break;
            case PASSWORD:
                // Connect using the entered password
                connect(WiFi.encryptionType(*selection), WiFi.SSID(*selection));
                break;
            case USERNAME:
                // Connect using the entered username and password
                *appState = PASSWORD; // Move to password entry state
                frame->fillSprite(TFT_BLACK);
                frame->setCursor(0, 20);
                frame->setTextColor(TFT_WHITE, TFT_BLACK);
                frame->println("Enter password for " + WiFi.SSID(*selection) + ":\n");
                renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
                frame->pushSprite(0, 0);
                break;
        }
        return;
    }
        if(key == ' ') {
        frame->fillSprite(TFT_BLACK);
        int x, y;
        centerText("Scanning for networks...", &x, &y);
        frame->drawString("Scanning for networks...", x, y);
        frame->pushSprite(0, 0);
        *numNetworks = WiFi.scanNetworks();
    }
    switch(*appState) {
        case SCANNING: // Scanning state
            if(key == 'w') (*selection)--;
            if(key == 's') (*selection)++;
            if(*selection > (*numNetworks)) *selection = *numNetworks - 1;
            else if(*selection == *numNetworks) *selection = 0;
            frame->fillSprite(TFT_BLACK);
            frame->setCursor(0, 20);
            if (*numNetworks == 0) {
                frame->println("no networks found");
            } else {
                frame->setTextColor(TFT_DARKGREEN, TFT_BLACK);
                frame->print(*numNetworks);
                frame->println(" networks found");
                for (int i = 4 * ((*selection) / 4); i < *numNetworks; ++i) {
                    if (i >= 4 * ((*selection) / 4) + 4) break; // Show only 4 networks at a time
                    // Print SSID and RSSI for each network found
                    if(i == *selection) frame->setTextColor(TFT_RED, TFT_BLACK);
                    else frame->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                    frame->print(i + 1);
                    frame->print(": ");
                    frame->print(WiFi.SSID(i));
                    drawBars(frame->getCursorY() - 8, WiFi.RSSI(i));
                    Serial.println(i);
                    frame->println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?"\n":"*\n");
                }
                frame->setTextColor(TFT_DARKCYAN, TFT_BLACK);
                frame->drawString("MAC: " + WiFi.macAddress(), 0, frame->height() - 15);
                renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
                frame->pushSprite(0, 0);
            }
            break;
        case PASSWORD: // Password entry state
            // Handle password input
            if(key == '\b') {
                if(credentials[CRED_PASSWORD].length() > 0)
                    credentials[CRED_PASSWORD].remove(credentials[CRED_PASSWORD].length() - 1);
            } else if(key != '^' && key != 7){ // Printable characters
                credentials[CRED_PASSWORD] += key;
            }
            frame->fillSprite(TFT_BLACK);
            frame->setCursor(0, 20);
            frame->setTextColor(TFT_WHITE, TFT_BLACK);
            frame->println("Enter password for " + WiFi.SSID(*selection) + ":\n");
            frame->println(credentials[CRED_PASSWORD]);
            renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
            frame->pushSprite(0, 0);
            break;
        case USERNAME: // Username entry state
            // Handle username input
            if(key == '\b') {
                if(credentials[CRED_USERNAME].length() > 0)
                    credentials[CRED_USERNAME].remove(credentials[CRED_USERNAME].length() - 1);
            } else if(key != '^' && key != 7) { // Printable characters
                credentials[CRED_USERNAME] += key;
            }
            frame->fillSprite(TFT_BLACK);
            frame->setCursor(0, 20);
            frame->setTextColor(TFT_WHITE, TFT_BLACK);
            frame->println("Enter username for " + WiFi.SSID(*selection) + ":\n");
            frame->println(credentials[CRED_USERNAME]);
            renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
            frame->pushSprite(0, 0);
            break;
        }
}
void WifiApp::run() {
    // This function can be used to handle any additional logic needed in the WiFi app
    // For now, it does nothing
    if(esp_timer_get_time() - *statusTimer > 1000000) { // Update every second
        *statusTimer = esp_timer_get_time();
        frame->fillRect(0, 0, tft.width(), 15, TFT_BLACK); // Clear the status bar area
        renderStatusBar().pushToSprite(frame, 0, 0, TFT_BLACK);
        frame->pushSprite(0, 0);
    }
}
void WifiApp::end() {
    // Cleanup when exiting the WiFi app
    free(statusTimer); // Free the timer memory
    free(selection); // Free the selection memory
    free(numNetworks); // Free the numNetworks memory
    free(appState); // Free the appState memory
    delete[] credentials; // Free the credentials array
    frame->deleteSprite(); // Delete the sprite
    delete frame; // Delete the sprite object
    taskHandler->releaseSemaphore("WiFi"); // Release the WiFi semaphore
}

void WifiApp::drawBars(uint16_t y, int32_t rssi) {
    // Draw the signal strength bars for each network
    frame->fillRoundRect(tft.width() - 30, y + 10, 6, 10, 2, TFT_RED); // Always have 1 bar
    //frame->drawString(String(rssi) + " dBm", tft.width() - 90, y);
    //placeholder for less signal, will get overwritten if more bars are needed
    frame->fillRoundRect(tft.width() - 21, y + 18, 4, 2, 2, TFT_YELLOW);
    frame->fillRoundRect(tft.width() - 13, y + 18, 4, 2, 2, TFT_GREEN);
    if(rssi >= -80) {
        frame->fillRoundRect(tft.width() - 22, y + 5, 6, 15, 2, TFT_YELLOW);
    }
    if(rssi >= -70) {
        frame->fillRoundRect(tft.width() - 14, y, 6, 20, 2, TFT_GREEN);
    }
}

void WifiApp::connect(wifi_auth_mode_t auth, String ssid) {
    // Function to connect to a WiFi network
    frame->fillSprite(TFT_BLACK);
    int x, y;
    centerText("Connecting to " + ssid + "...", &x, &y);
    frame->drawString("Connecting to " + ssid + "...", x, y);
    frame->pushSprite(0, 0);
    log_d("Connecting to %s, %s, %s|", ssid.c_str(), credentials[CRED_USERNAME].c_str(), credentials[CRED_PASSWORD].c_str());
    switch(auth) {
        case WIFI_AUTH_OPEN:
            WiFi.begin(ssid.c_str());
            break;
        case WIFI_AUTH_WEP:
        case WIFI_AUTH_WPA_PSK:
        case WIFI_AUTH_WPA2_PSK:
        case WIFI_AUTH_WPA_WPA2_PSK:
        case WIFI_AUTH_WPA3_PSK:
        case WIFI_AUTH_WPA2_WPA3_PSK:
            WiFi.begin(ssid.c_str(), credentials[CRED_PASSWORD].c_str());
            break;
        // case WIFI_AUTH_ENTERPRISE:
        //     WiFi.begin(ssid, WPA2_AUTH_PEAP, credentials[CRED_USERNAME].c_str(), credentials[CRED_USERNAME].c_str(), credentials[CRED_PASSWORD].c_str());
        //     break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            WiFi.begin(ssid.c_str(), WPA2_AUTH_TTLS, credentials[CRED_USERNAME].c_str(), credentials[CRED_USERNAME].c_str(), credentials[CRED_PASSWORD].c_str());
            break;
        default:
            frame->fillSprite(TFT_BLACK);
            centerText("Unsupported encryption type", &x, &y);
            frame->drawString("Unsupported encryption type", x, y);
            frame->pushSprite(0, 0);
            delay(2000);
            return;
    }
    uint8_t counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    counter++;
    if (counter > 220) { // Timeout after 15 seconds
        Serial.println("Connection timed out");
        frame->fillSprite(TFT_BLACK);
        centerText("Connection timed out", &x, &y);
        frame->drawString("Connection timed out", x, y);
        frame->pushSprite(0, 0);
        delay(1500);
        *appState = 0; // Reset state to scanning
        update(' ', true, false); // Rescan networks
        return;
        }
    }
    frame->fillSprite(TFT_BLACK);
    centerText("Connected! IP: " + WiFi.localIP().toString(), &x, &y);
    frame->drawString("Connected! IP: " + WiFi.localIP().toString(), x, y);
    frame->pushSprite(0, 0);
    WiFi.setAutoReconnect(true);
    Preferences preferences;
    preferences.begin("Messenger WiFi", false);
    String networkInfo = (auth == WIFI_AUTH_WPA2_ENTERPRISE) ? String(credentials[CRED_USERNAME] + "|" + credentials[CRED_PASSWORD]) : String(credentials[CRED_PASSWORD]);
    log_d("Storing network info: %s", networkInfo.c_str());
    preferences.putString(ssid.c_str(), networkInfo);
    preferences.end();
    delay(2000);
    *appState = 0; // Reset state to scanning
    update(' ', true, false); // Rescan networks
}