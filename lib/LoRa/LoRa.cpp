#include "LoRa.h"

/*
* Lora Handling Thread and helper functions
*
*Hermes message format:
* "HermesMeta:<senderID>:<recipientID>:<number of packets (1 byte)>"
* Followed by <number of packets> packets of:
* "HermesMsg:<packet number (1 byte)><messageData>"
*/

LoRaMessage* LoRa::packetReceiveQueue = nullptr;
LoRaMessage* LoRa::packetTransmitQueue = nullptr;
SX1262 LoRa::radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
TaskHandler* LoRa::taskHandler = nullptr;
volatile bool LoRa::interuptFlag = false;
String LoRa::radioID;

ICACHE_RAM_ATTR
void LoRa::radioInteruptAction() {
    //log_d("Interupt received");
    interuptFlag = true;
    radio.clearIrqFlags(radio.getIrqFlags() & (RADIOLIB_IRQ_SYNC_WORD_VALID | RADIOLIB_IRQ_PREAMBLE_DETECTED));
}

void LoRa::loraTask(void *pvParameters) {
    delay(2000);
    taskHandler = (TaskHandler *)pvParameters;
    // Initialize LoRa
    SPI.begin(11, 13, 12);
    radio.reset(true);
    {
    uint8_t state = radio.begin(LORA_FREQ);
    log_d("Lora.begin: %d", state);
    }
    radio.setDio2AsRfSwitch(true);
    radio.setSpreadingFactor(DEFAULT_SPREADING_FACTOR);
    radio.setBandwidth(DEFAULT_BANDWIDTH);
    radio.setCodingRate(DEFUALT_CODING_RATE);
    radio.setOutputPower(DEFAULT_OUTPUT_POWER);
    radio.setRxBoostedGainMode(true);
    radio.setTCXO(3.3); //TCXO will work fine at 3.3V (specific to hardware)
    radio.setPacketReceivedAction(radioInteruptAction);
    taskHandler->takeSemaphore("fs", portMAX_DELAY);
    File configFile = LittleFS.open("/config/settings.esp", "r");
    if (!configFile) {
        log_e("Failed to open settings file, using default ID of 'LoRa'");
        radioID = "LoRa";
    }
    else {
        while (configFile.available()) {
            // Read the radio ID from the file
            // Assuming the file contains a single line with the radio ID
            // Adjust this logic if the file format is different
        String line = configFile.readStringUntil('\n');
        if(line.startsWith("LoRaID:")) {
            radioID = line.substring(line.indexOf(':') + 1); // Extract the ID after "radioID:"
            log_d("Loaded radio ID: %s", radioID.c_str());
        } else {
             log_e("Invalid settings file format, using default ID of 'LoRa'");
            radioID = "LoRa";
        }
        log_d("Loaded radio ID: %s", radioID.c_str());
        }
    }
    configFile.close();
    taskHandler->releaseSemaphore("fs");
    radio.startReceive();
    log_d("LoRa initialized successfully");
    
    for (;;) {
        while(!taskHandler->takeSemaphore("radio", portMAX_DELAY));
        //RECIEVE
        // Check for incoming packets
        if (interuptFlag) {
            log_d("Handling received packet");
            interuptFlag = false;
            String payload = "";
            int state = radio.readData(payload);
            log_d("Received packet: %s, status: %d", payload.c_str(), state);
            if(state == RADIOLIB_ERR_NONE) {
                if(payload.startsWith("HermesMeta:")) {
                    //Valid packet, add to receive queue
                    payload = payload.substring(payload.indexOf(':') + 1); // Remove "Hermes:<senderID>:<recipientID>:"
                } else {
                    log_d("Somethings wrong with this packet, ignoring");
                    taskHandler->releaseSemaphore("radio");
                    continue; // Invalid packet format, ignore
                }
                String receiverID = payload.substring(0, payload.indexOf(':'));
                if(receiverID.equals(radioID) || receiverID.equals("all")) {
                    log_d("Packet for this device or broadcast, adding to queue");
                } else {
                    log_d("Packet not for this device, ignoring");
                    taskHandler->releaseSemaphore("radio");
                    continue;
                }
                payload = payload.substring(payload.indexOf(':') + 1); // Remove "<recipientID>:"
                String senderID = payload.substring(0, payload.indexOf(':'));
                payload = payload.substring(payload.indexOf(':') + 1); // Remove "<senderID>:"
                //we should only have the number of incoming packets left
                uint8_t numPackets = payload.charAt(0); // Convert
                //we know packet is valid and for us, add to queue
                String messageContent = receiveMessage(numPackets, senderID); // Remove packet number
                log_d("Parsed message from %s to %s: %s", senderID.c_str(), receiverID.c_str(), messageContent.c_str());
                LoRaMessage* newMessage = new LoRaMessage(messageContent, radio.getRSSI());
                newMessage->senderID = senderID;
                if (packetReceiveQueue == nullptr) {
                    packetReceiveQueue = newMessage;
                } else {
                    LoRaMessage* current = packetReceiveQueue;
                    while (current->next != nullptr) {
                        current = current->next;
                    }
                    current->next = newMessage;
                }
            }
        }
        //TRANSMIT
        cleanQueue();
        LoRaMessage* packet = packetTransmitQueue;
        while(packet != nullptr) {
            if(packet->status != LORA_UNSENT) {
                //log_d("Packet to %s already sent or in error state, skipping", packet->recipient.c_str());
                packet = packet->next;
                continue; // Skip already sent packets
            }
            uint8_t attempts = 0;
            while(attempts < LORA_RETRY_AMOUNT) { // Try to send the packet up to 2 times
            log_d("Transmitting packet to %s", packet->recipient.c_str());
            packet->status = transmitMessage(*packet);
            log_d("Transmit status: %d", packet->status);
            attempts++;
            if(packet->status == LORA_ERR_BUSY) {
                log_d("Radio is busy, retrying...");
                delay(2000); // Wait before retrying
                radio.standby();
                radio.clearIrqFlags(radio.getIrqFlags() & (RADIOLIB_IRQ_SYNC_WORD_VALID | RADIOLIB_IRQ_PREAMBLE_DETECTED));
                
            }
            if(packet->status == RADIOLIB_ERR_NONE) {
                log_d("Packet transmitted successfully");
                break; // Successful transmission
            }
        }
            packet = packet->next;
        }
        taskHandler->releaseSemaphore("radio");
        vTaskDelay(100 / portTICK_PERIOD_MS); // Polling delay
    }
}

bool LoRa::isReceiving() {
    uint16_t flags = radio.getIrqFlags();
    return (flags & (RADIOLIB_IRQ_SYNC_WORD_VALID | RADIOLIB_IRQ_PREAMBLE_DETECTED));
    //Might revisit this, could return false positives
}

int LoRa::transmitMessage(LoRaMessage& packet) {
    if(isReceiving()) {
        log_d("Radio is currently receiving, cannot transmit, retrying...");
        return LORA_ERR_BUSY;
    }
    log_d("Not reciving, preparing to transmit");
    String metadata = "HermesMeta:" + packet.recipient + ":" + radioID + ":";
    const int maxDataLength = 254 - String("HermesMsg:").length();
    const uint8_t amountOfPackets = (packet.msg.length() / maxDataLength) + 1;
    metadata += (char)amountOfPackets;
    //radio.clearDio1Action();
    int state = radio.startTransmit(metadata);
     while(!interuptFlag) yield(); // Wait for transmission to complete
     interuptFlag = false;
    if(state != RADIOLIB_ERR_NONE) return state;
    uint8_t packetNumber = 0;
    String chunks[amountOfPackets]; //the stack is my best friend
    while(packet.msg.length() > maxDataLength) {
            chunks[packetNumber++] = packet.msg.substring(0, maxDataLength);
            packet.msg.remove(0, maxDataLength);
    }
    chunks[packetNumber++] = packet.msg; // Last chunk
    packetNumber = 1; // Reset for transmission
    log_d("Transmitting packet to %s, total length %d, will be sent in %d packets", packet.recipient.c_str(), packet.msg.length(), amountOfPackets);
        //send all packets
    while(packetNumber <= amountOfPackets) {
        //The first byte of the message data is not being sent?
        String tmp = "HermesMsg:";
        tmp += char(packetNumber);
        tmp += chunks[packetNumber - 1];
        log_d("Sending packet %d: %s", packetNumber, tmp.c_str());
        state = radio.startTransmit(tmp);
        packetNumber++;
        while(!interuptFlag) yield(); // Wait for transmission to complete
        interuptFlag = false;
        if(state != RADIOLIB_ERR_NONE) return state;
        delay(100); //give a "cooldown" period for the radio
    }
    //all packets sent, wait for acknowledgment
    radio.setDio1Action(radioInteruptAction);
    radio.startReceive();
    uint64_t timeCounter = 0;
    while(timeCounter < RECIEVE_TIMEOUT) { 
        if(interuptFlag) {
            interuptFlag = false;
            String tmp = "";
            int state = radio.readData(tmp);
            if(tmp.startsWith("Hermes:REQ:" + radioID)) { //we got something back, but we missed a packet
                packetNumber = tmp.charAt(tmp.length() - 1); // Convert char to int
                state = radio.startTransmit("HermesMsg:" + char(packetNumber) + chunks[packetNumber - 1]);
                while(!interuptFlag) yield(); // Wait for transmission to complete
                interuptFlag = false;
                if(state != RADIOLIB_ERR_NONE) return state;
                timeCounter = 0; // Reset timer to wait for missing packet
            }
            else if(tmp.equals("Hermes:ACK:" + radioID)) {
                log_d("Acknowledgment received for transmission to %s, clean up and return", packet.recipient.c_str());
                radio.setDio1Action(radioInteruptAction);
                radio.startReceiveDutyCycleAuto(); // Return to receive mode
            return RADIOLIB_ERR_NONE;
        }  // Acknowledgment received
    }
        timeCounter++;
        vTaskDelay(1 / portTICK_PERIOD_MS); // Polling delay
    }
    log_e("No acknowledgment received, giving up");
    radio.standby();
    radio.clearIrqFlags(radio.getIrqFlags() & (RADIOLIB_IRQ_SYNC_WORD_VALID | RADIOLIB_IRQ_PREAMBLE_DETECTED));
    radio.setDio1Action(radioInteruptAction);
    radio.startReceiveDutyCycleAuto(); // Ensure radio is in receive mode
    return LORA_ERR_NO_ACK;
}

String LoRa::receiveMessage(uint8_t numPackets, String from) {
    String payloads[numPackets];
    uint64_t timeCounter = 0;
    int16_t waitingOn = -2; //-1 means we have all packets, -2 means we havent requested any packets back yet
    while(timeCounter <= RECIEVE_TIMEOUT && waitingOn != -1) {
            if(interuptFlag) {
                interuptFlag = false;
                timeCounter = 0; // Reset the timer if a packet is received
                String payload;
                int state = radio.readData(payload);
                if(state == RADIOLIB_ERR_NONE) {
                    if(!payload.startsWith("HermesMsg:")) continue; // Invalid packet format, ignore
                    log_d("%s", payload.c_str());
                    payload = payload.substring(payload.indexOf(':') + 1); // Remove "HermesMsg:"
                    uint8_t packetNumber = payload.charAt(0); // Convert char to int
                    payload = payload.substring(payload.indexOf(':') + 1); // Remove packet number
                    if(packetNumber > numPackets) {
                        log_w("Received packet number %d, but only expecting %d packets", packetNumber, numPackets);
                        continue; // Ignore packets that are out of range
                    }
                    payloads[packetNumber - 1] = payload.substring(1); // Remove packet number
                    log_d("Received packet %d: %s", packetNumber, payload.c_str());
                    if(packetNumber == numPackets || packetNumber == waitingOn) {
                        log_d("Received last packet");
                        for(int i = 0; i < numPackets; i++) {
                            if(payloads[i].equals("")) {
                                log_w("Missing packet %d, asking for it", i + 1);
                                radio.startTransmit("Hermes:REQ:" + from + ":" + char(i + 1));
                                //clear transmit interrupt
                                while(!interuptFlag) yield(); // Wait for transmission to complete
                                interuptFlag = false;
                                timeCounter = 0; // Reset timer to wait for missing packet
                                waitingOn = i + 1;
                                break;
                            } //TODO: Implement a way to ask for missing packets
                            else if(i == numPackets - 1) waitingOn = -1; // We have all packets
                        }
                    }
            }
        }
        timeCounter++;
        vTaskDelay(1);
    }
    //check for timeout, if we timed out return nothing
    if(timeCounter >= RECIEVE_TIMEOUT) return "";
    String final = "";
    for(int i = 0; i < numPackets; i++) final += payloads[i];
    radio.clearDio1Action();
    radio.transmit("Hermes:ACK:" + from); //Tell the transmitter we got the message, return them to receive mode
    interuptFlag = false;
    radio.setDio1Action(radioInteruptAction);
    radio.startReceiveDutyCycleAuto();
    return final;
}

void LoRa::changeSettings(float freq, float bandwidth, uint8_t spreadingFactor, uint8_t codingRate) {
    log_d("Changing LoRa settings: Freq: %.2f MHz, Bandwidth: %d kHz, Spreading Factor: %d, Coding Rate: %d", freq, bandwidth, spreadingFactor, codingRate);
    radio.setFrequency(freq);
    radio.setBandwidth(bandwidth);
    radio.setSpreadingFactor(spreadingFactor);
    radio.setCodingRate(codingRate);
}

void LoRa::radioSleep() {
    taskHandler->takeSemaphore("radio", portMAX_DELAY);
    radio.sleep();
    taskHandler->releaseSemaphore("radio");
}

void LoRa::cleanQueue() {
    // Clean up any acknowledged packets from the transmit queue
    if(packetTransmitQueue == nullptr) return; //no packets in queue

    //delete all packets at the front of the queue that have been acknowledged, stop at the first packet that is pending transmission or if we reach the end of the queue
    while(packetTransmitQueue->status == LORA_SENT_ACK) {
        log_d("Cleaning up acknowledged packet %x", packetTransmitQueue);
        LoRaMessage* temp = packetTransmitQueue;
        packetTransmitQueue = packetTransmitQueue->next;
        delete temp;
        if(packetTransmitQueue == nullptr) return;
    }

    if(packetTransmitQueue->next == nullptr) return; //only one packet in queue, and its pending or in error, so we can stop here
    //delete packets after the first pending packet that have been acknowledged
    LoRaMessage* current = packetTransmitQueue;
    while(current->next != nullptr && current->next->status != LORA_UNSENT) {
        if(current->next->status == LORA_SENT_ACK) {
            LoRaMessage* temp = current->next;
            current->next = current->next->next;
            delete temp;
        } else {
            current = current->next;
        }
    }
}