#include "home.h"

const bool FORWARD = true;
const bool BACKWARDS = false;
const uint8_t ICON_SIZE = 156;
const int ICON_CENTER_X = 82;
const int ICON_CENTER_Y = 7;
const int ANIMATION_STEP = 15;

void Home::animateIcons(bool direction) {
    TFT_eSprite bar = renderStatusBar();
    if(direction == FORWARD) {
        int last = appState++;
        if(appState == APP_COUNT) appState = 0;
        for(int y = ICON_CENTER_Y - ANIMATION_STEP; y + ICON_SIZE + ICON_CENTER_Y > ICON_CENTER_Y; y -= ANIMATION_STEP) {
            bkg.pushToSprite(&frame, 0, 0);
            icons[last].sprite.pushToSprite(&frame, ICON_CENTER_X, y, TFT_BLACK);
            icons[appState].sprite.pushToSprite(&frame, ICON_CENTER_X, y + ICON_CENTER_Y + ICON_SIZE, TFT_BLACK);
            bar.pushToSprite(&frame, 0, 0, TFT_BLACK);
            frame.pushSprite(0, 0);
        }
    }
    else {
        int last = appState--;
        if(appState < 0) appState = APP_COUNT - 1;
        for(int y = ICON_CENTER_Y + ANIMATION_STEP; y < ICON_CENTER_Y + ICON_SIZE; y += ANIMATION_STEP) {
            bkg.pushToSprite(&frame, 0, 0);
            icons[last].sprite.pushToSprite(&frame, ICON_CENTER_X, y, TFT_BLACK);
            icons[appState].sprite.pushToSprite(&frame, ICON_CENTER_X, y - ICON_CENTER_Y - ICON_SIZE, TFT_BLACK);
            bar.pushToSprite(&frame, 0, 0, TFT_BLACK);
            frame.pushSprite(0, 0);
        }
    }
}



void Home::loadIcons() {
    //icons are 96x96 pixels
    //give 30 pixel padding on each side horizontal, and 60 pixels on the bottom for text
    for(int i = 0; i < APP_COUNT; i++) {
        icons[i].sprite.createSprite(ICON_SIZE, ICON_SIZE);
        icons[i].sprite.fillSprite(TFT_BLACK);
        renderPNG(icons[i].location).pushToSprite(&icons[i].sprite, 30, 0, TFT_BLACK); //icons sprites have 30 pixel padding
        icons[i].sprite.loadFont(FONT_SMALL);
        icons[i].sprite.drawString(icons[i].name, (icons[i].sprite.width() - icons[i].sprite.textWidth(icons[i].name)) / 2, 126); //96+30 pixel padding from image to text
        icons[i].sprite.unloadFont();
    }

}

void Home::unloadIcons() {
    for(int i = 0; i < APP_COUNT; i++) {
            icons[i].sprite.deleteSprite();
        }
}

void Home::update(char key, bool pressed, bool alt) {
    if(!pressed) return;
    switch(key) {
        case 'w':
        case 'W':
        animateIcons(BACKWARDS);
        bkg.pushToSprite(&frame, 0, 0);
        icons[appState].sprite.pushToSprite(&frame, ICON_CENTER_X, ICON_CENTER_Y, TFT_BLACK);
        renderStatusBar().pushToSprite(&frame, 0, 0, TFT_BLACK);
        frame.pushSprite(0, 0);
        break;
        case 's':
        case 'S':
        animateIcons(FORWARD);
        bkg.pushToSprite(&frame, 0, 0);
        icons[appState].sprite.pushToSprite(&frame, ICON_CENTER_X, ICON_CENTER_Y, TFT_BLACK);
        renderStatusBar().pushToSprite(&frame, 0, 0, TFT_BLACK);
        frame.pushSprite(0, 0);
        break;
        case '\n':
            currentApp = appState + 1; 
        break;
    }
}

void Home::run() {
    // This function can be used to handle any additional logic needed in the home app
    if(esp_timer_get_time() - *statusTimer > 1000000) { // Update every second
        *statusTimer = esp_timer_get_time(); // Clear the status bar area
        TFT_eSprite bar = TFT_eSprite(&tft);
        bar.createSprite(tft.width(), 15);
        bkg.pushToSprite(&bar, 0, 0);
        while(!taskHandler->takeSemaphore("status", portMAX_DELAY));
        renderStatusBar().pushToSprite(&bar, 0, 0, TFT_BLACK);
        taskHandler->releaseSemaphore("status");
        bar.pushSprite(0, 0);
        bar.deleteSprite();
    }
}

void Home::start() {
    // Initial setup and frame for the home app
    appState = 0;
    statusTimer = new uint64_t;
    *statusTimer = 0;
    while(!taskHandler->takeSemaphore("fs", portMAX_DELAY)); // Ensure filesystem is ready
    loadIcons();
    taskHandler->releaseSemaphore("fs");
    tft.loadFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    frame.createSprite(tft.width(), tft.height());
    bkg.pushToSprite(&frame, 0, 0);
    icons[appState].sprite.pushToSprite(&frame, ICON_CENTER_X, ICON_CENTER_Y, TFT_BLACK);
    while(!taskHandler->takeSemaphore("status", portMAX_DELAY));
    renderStatusBar().pushToSprite(&frame, 0, 0, TFT_BLACK);
    taskHandler->releaseSemaphore("status");
    frame.pushSprite(0, 0);
}


void Home::end() {
    // Cleanup when exiting the home app
    unloadIcons();
    frame.deleteSprite();
    delete statusTimer; // Free the timer memory
    statusTimer = nullptr; // Set to nullptr to avoid dangling pointer
}
