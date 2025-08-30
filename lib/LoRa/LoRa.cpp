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
int LoRa::lastTransmitStatus = 69;
SX1262 LoRa::radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
TaskHandler* LoRa::taskHandler = nullptr;
bool LoRa::interuptFlag = false;
String LoRa::radioID;

ICACHE_RAM_ATTR
void LoRa::radioInteruptAction() {
    log_d("Packet received");
    interuptFlag = true;
    radio.clearIrqFlags(radio.getIrqFlags() & (RADIOLIB_IRQ_SYNC_WORD_VALID | RADIOLIB_IRQ_PREAMBLE_DETECTED));
}

void LoRa::loraTask(void *pvParameters) {
    delay(2000);
    taskHandler = (TaskHandler *)pvParameters;
    // Initialize LoRa
    SPI.begin(11, 13, 12);
    radio.reset(true);
    log_d("Lora.begin: %d", radio.begin(LORA_FREQ));
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
        if(packetTransmitQueue != nullptr) {
            LoRaMessage* packet = packetTransmitQueue;
            packetTransmitQueue = packetTransmitQueue->next; // Remove from queue
            log_d("Transmitting packet to %s", packet->recipient.c_str());
            lastTransmitStatus = transmitMessage(*packet);
            log_d("Transmit status: %d", lastTransmitStatus);
            if(lastTransmitStatus != RADIOLIB_ERR_NONE) {
                log_e("Failed to transmit packet to %s, retrying...", packet->recipient.c_str());
            } else {
                log_d("Packet transmitted successfully");
            }
            delete packet; // Free memory after successful transmission
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
    int retryCount = 0;
    while(isReceiving()) {
        log_d("Radio is currently receiving, cannot transmit, retrying...");
        vTaskDelay(RETRY_SEND_MS / portTICK_PERIOD_MS);
        if(++retryCount >= RETRY_AMOUNT) {
            log_e("Failed to transmit packet after %d attempts", RETRY_AMOUNT);
            radio.clearIrqFlags(radio.getIrqFlags() & (RADIOLIB_IRQ_SYNC_WORD_VALID | RADIOLIB_IRQ_PREAMBLE_DETECTED));
            //return LORA_ERR_BUSY;
        }
    }
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
    log_d("Transmitting packet to %s, total length %d, will be sent in %d packets", packet.recipient.c_str(), packet.msg.length(), amountOfPackets);
    while(packet.msg.length() > maxDataLength) {
        String chunk = packet.msg.substring(0, maxDataLength);
        packet.msg.remove(0, maxDataLength);
        state = radio.startTransmit("HermesMsg:" + char(packetNumber) + chunk);
        while(!interuptFlag) yield(); // Wait for transmission to complete
        interuptFlag = false;
        if(state != RADIOLIB_ERR_NONE) return state;
        delay(100); //give a "cooldown" period for the radio
    }
        state = radio.startTransmit("HermesMsg:" + char(packetNumber) + packet.msg);
        while(!interuptFlag) yield(); // Wait for transmission to complete
        interuptFlag = false;
        log_d("Waiting for acknowledgment for packet %d", packetNumber);
        if(state != RADIOLIB_ERR_NONE) return state;
    //wait for acknowledgment
    radio.setDio1Action(radioInteruptAction);
    radio.startReceive();
    
    uint64_t timeCounter = 0;
    while(timeCounter < 3000) { // Wait for 3 seconds for acknowledgment
        if(interuptFlag) {
            interuptFlag = false;
            String tmp;
            int state = radio.readData(tmp);
            return RADIOLIB_ERR_NONE;
        }  // Acknowledgment received
        timeCounter++;
        vTaskDelay(1 / portTICK_PERIOD_MS); // Polling delay
    }
    log_e("No acknowledgment received for packet %d, giving up", packetNumber);
    radio.standby();
    radio.clearIrqFlags(radio.getIrqFlags() & (RADIOLIB_IRQ_SYNC_WORD_VALID | RADIOLIB_IRQ_PREAMBLE_DETECTED));
    radio.startReceiveDutyCycleAuto(); // Ensure radio is in receive mode
    return LORA_ERR_NO_ACK;
}

String LoRa::receiveMessage(uint8_t numPackets, String recipient) {
    String payloads[numPackets];
    uint64_t timeCounter = 0;
    while(timeCounter <= 5000) {
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
                    payload = payload.substring(payload.indexOf(':')); // Remove packet number
                    if(packetNumber > numPackets) {
                        log_w("Received packet number %d, but only expecting %d packets", packetNumber, numPackets);
                        continue; // Ignore packets that are out of range
                    }
                    payloads[packetNumber - 1] = payload.substring(1); // Remove packet number
                    log_d("Received packet %d: %s", packetNumber, payload.c_str());
                    if(packetNumber == numPackets) {
                        log_d("Received last packet");
                        for(int i = 0; i < numPackets; i++) {
                            if(payloads[i].equals("")) delay(1); //TODO: Implement a way to ask for missing packets
                        }
                        break;
                    }
            }
        }
        timeCounter++;
        vTaskDelay(1);
    }
    //check for timeout, if we timed out return nothing
    if(timeCounter == 100000) return "";
    String final = "";
    for(int i = 0; i < numPackets; i++) final += payloads[i];
    radio.clearDio1Action();
    //radio.transmit("Hermes:ACK:" + recipient);
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
}