#include "TaskHandler.h"
#include "LoRa.h"


TaskHandler taskHandler = TaskHandler();

void setup() {
    Serial.begin(115200);
    delay(2000);
    log_d("Starting LoRa Test");
    taskHandler.addSemaphore("radio");
    taskHandler.addTask(LoRa::loraTask, "LoRa", 4096, 1, 1);
    taskHandler.startTask("LoRa");
}

void loop() {
    //Main loop checks serial interface, all work is done in LoRa task
    
    if(Serial.available()) {
        String input = Serial.readStringUntil('\n');
        if(input.startsWith("send ")) {
            String message = input.substring(5, input.length() - 1);
            LoRaMessage* msg = new LoRaMessage(message.substring(message.indexOf(',') + 1)); // Extract message content
            msg->recipient = message.substring(0, message.indexOf(',')); // Extract recipient from message
            LoRa::packetTransmitQueue = msg;
            Serial.printf("Queued message for %s: %s\n", msg->recipient.c_str(), message.c_str());
            while(msg->status == LORA_UNSENT) {
                delay(100);
            }
            Serial.printf("Message status: %d\n", msg->status);
            msg->status = LORA_SENT_ACK;
        }
    }

    if(LoRa::packetReceiveQueue != nullptr) {
        LoRaMessage* msg = LoRa::packetReceiveQueue;
        LoRa::packetReceiveQueue = LoRa::packetReceiveQueue->next;
        Serial.print("Received message from ");
        Serial.print(msg->senderID);
        Serial.print(": ");
        Serial.print(msg->msg);
        Serial.print(" | RSSI: ");
        Serial.println(msg->rssi);
        //send something back
        if(msg->msg.equals("ping")) {
        LoRaMessage* response = new LoRaMessage{"pong"};
        response->recipient = msg->senderID;
        Serial.println(msg->senderID + "|" + msg->senderID.length());
        LoRa::packetTransmitQueue = response;
        delete msg;
        Serial.println("Sent back");
        }
    }
}