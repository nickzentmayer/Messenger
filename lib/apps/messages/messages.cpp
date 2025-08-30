#include "messages.h"

void MessagesApp::start() {
    statusTimer = new uint64_t;
    recipientID = new String();
    message = new String();
    appState = new int();
    frame.createSprite(tft.width(), tft.height());
    frame.loadFont(FONT_SMALL);
    frame.setTextColor(TFT_WHITE, TFT_BLACK);
}

void MessagesApp::update(char key, bool pressed, bool alt) {
    if(!pressed) return;
    switch(key) {
        case '&':
        case 0:
        case '^':
        break;

        // Handle key presses for the messages app
        case '\n': // Example key to send a message
            // Implement message sending logic here
            if(*appState == 0) *appState = 1;
            else {
                LoRa::lastTransmitStatus = 69;
                LoRaMessage* newMessage = new LoRaMessage{*message};
                newMessage->recipient = *recipientID;

                if(LoRa::packetTransmitQueue == nullptr) LoRa::packetTransmitQueue = newMessage;
                else {
                    LoRaMessage* current = LoRa::packetTransmitQueue;
                    while(current->next != nullptr) current = current->next;
                    current->next = newMessage;
                }
                tft.fillScreen(TFT_BLACK);
                int x, y;
                centerText("Sent '" + *message + "'!", &x, &y);
                tft.drawString("Sent '" + *message + "'!", x, y);
                while(LoRa::lastTransmitStatus == 69) delay(1);
                centerText("Status: " + String(LoRa::lastTransmitStatus), &x, &y);
                tft.drawString("Status: " + String(LoRa::lastTransmitStatus), x, y + 20);
                delay(2000);
                frame.fillScreen(TFT_BLACK);
                *appState = 0;
            }
            break;
        case '<':
            if(*appState == 0) *recipientID = recipientID->substring(0, recipientID->length() - 1);
            else if(*appState == 1) *message = message->substring(0, message->length() - 1);
            break;
        default:
            if(*appState == 0) *recipientID += key;
            else if(*appState == 1) *message += key;
            break;
    }
    frame.setCursor(0, 30);
    frame.fillSprite(TFT_BLACK);
    if(*appState == 0) {
        frame.println("Enter recipient ID:\n");
        frame.print(*recipientID);
    }
    else {
        frame.println("Enter message:\n");
        frame.print(*message);
    }
    frame.fillRect(60, 0, tft.width() - 60, 15, TFT_BLACK); // Clear the status bar area
    renderStatusBar().pushToSprite(&frame, 0, 0, TFT_BLACK);
    frame.pushSprite(0, 0);
}

void MessagesApp::run() {
    if(esp_timer_get_time() - *statusTimer > 1000000) { // Update every second
        *statusTimer = esp_timer_get_time();
        frame.fillRect(60, 0, tft.width() - 60, 15, TFT_BLACK); // Clear the status bar area
        renderStatusBar().pushToSprite(&frame, 0, 0, TFT_BLACK);
        frame.pushSprite(0, 0);
    }
    if(LoRa::packetReceiveQueue != nullptr) {
        // Process received packets
        LoRaMessage* packet = LoRa::packetReceiveQueue;
        LoRa::packetReceiveQueue = LoRa::packetReceiveQueue->next; // Remove from queue
        // Display received message
        frame.fillSprite(TFT_BLACK);
        int x, y;
        centerText("Received: " + packet->msg, &x, &y);
        frame.drawString("Received: " + packet->msg, x, y);
        centerText("From: " + packet->senderID, &x, &y);
        frame.drawString("From: " + packet->senderID, x, y + 20);
        centerText("RSSI: " + String(packet->rssi), &x, &y);
        frame.drawString("RSSI: " + String(packet->rssi), x, y + 40);
        // Wait for a second before clearing the message
        frame.pushSprite(0, 0);
        delay(3000);
        delete packet;
    }
}

void MessagesApp::end() {
    delete statusTimer;
    delete recipientID;
    delete message;
    frame.deleteSprite();
}