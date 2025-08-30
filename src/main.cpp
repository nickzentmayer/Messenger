#include <Arduino.h>
#include <time.h>
#include "TaskHandler.h"
#include "taskHandler.h"
#include "Keyboard.h"
#include "power.h"
#include "GUI.h"
#include "GPS.h"
#include "LoRa.h"


//Task Handling
TaskHandler taskHandler = TaskHandler();



void setup() {
    pinMode(0, INPUT);
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
  int waitTime = 0;
  while(waitTime < 500) {
    if(digitalRead(0)) {
      //esp_sleep_get_ext1_wakeup_status
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
      esp_deep_sleep_start();
    }
    delay(1);
    waitTime += 1;
  }
}
    #ifdef DEBUG
    //delay(2000);
    log_d("Running...");
    log_d("HERMES OS %s", "0.0.0");
    #endif
    //time
    configTime(3600*TIMEZONE, DAYSAVETIME*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
    //Initialize the File system
        //FS Init
    if (!LittleFS.begin(false)) {
        log_e("Failed to mount file system");
        return;
    } else {
        log_i("File system mounted successfully");
    }
    //taskHandler.addSemaphore("i2c");
    taskHandler.addSemaphore("fs");
    taskHandler.addSemaphore("keyboardRW");
    taskHandler.addSemaphore("powerSemaphore");
    taskHandler.addSemaphore("status");
    taskHandler.addSemaphore("gps");
    taskHandler.addSemaphore("radio");


    taskHandler.addTask(pwrTask, "Power Task");
    taskHandler.addTask(guiTask, "GUI Task", 30 * 1024, 2, 1); // GUI task with higher priority
    taskHandler.addTask(Keyboard::keyboardTask, "Keyboard Task"); // Keyboard task handle will be set later
    taskHandler.addTask(GPS::gpsTask, "GPS Task");
    taskHandler.addTask(LoRa::loraTask, "LoRa", 4096, 1, 1);

    // Start the tasks
    taskHandler.startTask("Power Task");
    taskHandler.startTask("GUI Task");
    taskHandler.startTask("Keyboard Task");
    taskHandler.startTask("GPS Task");
    taskHandler.startTask("LoRa");

    vTaskDelete(NULL);
  }

void loop() {
  // This is a placeholder for the main loop.
  // You can add your code here to run repeatedly.
  
}