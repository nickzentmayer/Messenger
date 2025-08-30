#pragma once
#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include "TaskHandler.h"
#include <RadioLib.h>
#include <LittleFS.h>

#define LORA_CS 4
#define LORA_RST 10
#define LORA_BUSY 17
#define LORA_DIO1 16
#define LORA_FREQ 915.0 // Frequency in Hz
#define DEFUALT_CODING_RATE 8
#define DEFAULT_SPREADING_FACTOR 12
#define DEFAULT_BANDWIDTH 125.0
#define DEFAULT_OUTPUT_POWER 22 // Output power in dBm

#define RETRY_SEND_MS 5000 // Retry sending a packet every second if it fails
#define RETRY_AMOUNT 2 // Number of retries before giving up

#define LORA_ERR_BUSY 2 //Airspace is busy
#define LORA_ERR_NO_ACK 3 //No acknowledgment received for the packet

struct LoRaMessage {
    String senderID;
    String recipient;
    String msg;
    int rssi;
    LoRaMessage* next;
    LoRaMessage(String message) : msg(message), next(nullptr) {}
    LoRaMessage(String message, int r) : msg(message), rssi(r), next(nullptr) {}
    LoRaMessage() : senderID(""), recipient(""), msg(""), rssi(0), next(nullptr) {}
};

class LoRa {
public:
    static void loraTask(void *pvParameters);
    static LoRaMessage* packetReceiveQueue;
    static LoRaMessage* packetTransmitQueue;
    static void changeSettings(float freq, float bandwidth, uint8_t spreadingFactor, uint8_t codingRate);
    static bool isReceiving();
    static int transmitMessage(LoRaMessage& packet);
    static String receiveMessage(uint8_t numPackets, String radioID);
    static void radioSleep();
    static int lastTransmitStatus;
private:
    LoRa() = delete; // Prevent instantiation
    static void radioInteruptAction();
    static SX1262 radio;
    static TaskHandler* taskHandler;
    static bool interuptFlag;
    static String radioID;
};

#endif