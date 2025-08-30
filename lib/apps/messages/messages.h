#pragma once
#ifndef MESSAGES_H
#define MESSAGES_H

#include "app.h"
#include "LoRa.h"

class MessagesApp : public AppBase {
    public:
    MessagesApp(TaskHandler* handler) : AppBase(handler) {}
    void start();
    void update(char key, bool pressed, bool alt);
    void run();
    void end();
    private:
    uint64_t* statusTimer; // Timer to manage status updates
    TFT_eSprite frame = TFT_eSprite(&tft);
    String* recipientID; // ID of the recipient for sending messages
    String* message;
    int* appState;
};

#endif