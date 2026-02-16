#include "power.h"

float Power::batteryVoltage = 0.0;
float Power::batteryPercent = 0.0;
float Power::chargeRate = 0.0;
TaskHandler* Power::taskHandler = nullptr;

const uint8_t initRetrys = 3;

void Power::devicesOn() {
    pinMode(LDO_EN, OUTPUT);
    digitalWrite(LDO_EN, HIGH);
    ledcSetup(0, 5000, 8); // Channel 0, 5kHz frequency, 8-bit resolution
    ledcAttachPin(KEY_BACKLIGHT, 0); // Channel 0, 5kHz frequency, 8-bit resolution
    ledcAttachPin(TFT_BACKLIGHT, 0); // Channel 0, 5kHz frequency, 8-bit resolution
    ledcWrite(0, 255); // Set the backlight to maximum brightness (inverse logic)
}

void Power::devicesOff() {
    digitalWrite(LDO_EN, LOW);
}
void Power::changeBacklight(uint8_t brightness) {
    // Set the backlight brightness using PWM
    ledcWrite(0, 255 - brightness); // Inverse logic for brightness control
}
void Power::beginSleep() {
    //lora.sleep();
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
    //   pinMode(KEY_BACKLIGHT, OUTPUT);
    //   digitalWrite(KEY_BACKLIGHT, LOW);
    //   gpio_hold_en(KEY_BACKLIGHT);
    //   gpio_hold_en(LDO_EN);
    //   gpio_deep_sleep_hold_en();
      delay(100);
      esp_deep_sleep_start();
}
void Power::pwrTask(void *pvParameters) {
    taskHandler = (TaskHandler *)pvParameters;
    uint16_t timeCounter = 0;
    uint64_t batteryTimer = 0;
    bool batteryConnected = false;

    pinMode(0, INPUT);
    devicesOn();
    Wire.setPins(1, 2);
    Wire.begin();
    Adafruit_MAX17048 maxlipo;
    taskHandler->takeSemaphore("i2c", portMAX_DELAY);
    for(int i = 0; i < initRetrys; ++i) {
        if (maxlipo.begin(&Wire, false)) {
            batteryConnected = true;
            break;
        }
        log_w("Couldn't find MAX17048? Retrying... (%d/%d)", i + 1, initRetrys);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (!batteryConnected) {
        log_e("Couldn't find MAX17048?\nMake sure a battery is plugged in!");
        batteryPercent = -1.0;
        batteryVoltage = -1.0;
    } 
    else {
        batteryPercent = (maxlipo.cellPercent() > 100.0) ? 100.0 : maxlipo.cellPercent();
        batteryVoltage = maxlipo.cellVoltage();
        chargeRate = maxlipo.chargeRate();
    }
    taskHandler->releaseSemaphore("i2c");
    log_d("Powered Up");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    changeBacklight(255); // Turn backlight to max when key is pressed
    for(;;){
        while(!digitalRead(0) && timeCounter < 500) {
            timeCounter++;
            log_d("PWR KEY HELD");
            vTaskDelay(1);
        } 
        while(timeCounter >= 500) {
            
        //make sure no tasks are doing important work
            if (taskHandler->takeSemaphore("powerSemaphore", portMAX_DELAY)) {
                //TODO: Save state and end all tasks
                LoRa::radioSleep();
                devicesOff();
                beginSleep();
            }
        }
        if(batteryConnected) {
            if(batteryTimer >= 5000) { 
                taskHandler->takeSemaphore("status", portMAX_DELAY);
                batteryPercent = (maxlipo.cellPercent() > 100.0) ? 100.0 : maxlipo.cellPercent();
                batteryVoltage = maxlipo.cellVoltage();
                chargeRate = maxlipo.chargeRate();
                taskHandler->releaseSemaphore("status");
                //log_d("Battery percent: %s", String(batteryPercent).c_str());
                //log_d("Battery voltage: %s", String(batteryVoltage).c_str());
                //log_d("Charge rate: %s", String(chargeRate).c_str());
                if (batteryVoltage < 3.3) {
                    log_w("Battery low, entering sleep mode");
                    taskHandler->takeSemaphore("powerSemaphore", portMAX_DELAY);
                    devicesOff();
                    beginSleep();
                }
                batteryTimer = 0;
            }
            ++batteryTimer;
        }
        timeCounter = 0;
        vTaskDelay(1);
    }
}