#include <Arduino.h>
#include <time.h>
#include "TaskHandler.h"
#include "Keyboard.h"
#include "power.h"
#include "GPS.h"
#include "LoRa.h"
#include "GUI.h"
#include <Preferences.h>


#define CONFIG_SPIRAM_SUPPORT 1

//Task Handling
TaskHandler taskHandler = TaskHandler();
Preferences preferences;


void setup() {
    pinMode(0, INPUT);
    esp_log_set_vprintf(&ramLog);
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
    esp_log_level_set("*", (esp_log_level_t)4);
    log_d("Running...");
    log_d("HERMES OS %s", "0.0.0");
    Serial.begin(115200);
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
    preferences.begin("Messenger WiFi", true); // Read-only mode

    taskHandler.addSemaphore("i2c");
    taskHandler.addSemaphore("fs");
    taskHandler.addSemaphore("keyboardRW");
    taskHandler.addSemaphore("powerSemaphore");
    taskHandler.addSemaphore("status");
    taskHandler.addSemaphore("gps");
    taskHandler.addSemaphore("radio");
    taskHandler.addSemaphore("WiFi");

    taskHandler.addTask(Power::pwrTask, "Power Task");
    taskHandler.addTask(GUI::guiTask, "GUI Task", 30 * 1024, 2, 1); // GUI task with higher priority
    taskHandler.addTask(Keyboard::keyboardTask, "Keyboard Task", 4096, 1, 0); // Keyboard task handle will be set later
    taskHandler.addTask(GPS::gpsTask, "GPS Task");
    taskHandler.addTask(LoRa::loraTask, "LoRa", 4096, 1, 1);

    // Start the tasks
    taskHandler.startTask("Power Task");
    taskHandler.startTask("GUI Task");
    taskHandler.startTask("Keyboard Task");
    taskHandler.startTask("GPS Task");
    taskHandler.startTask("LoRa");

    //vTaskDelete(NULL);
    WiFi.enableSTA(true);
  }

void loop() {
  //Loop will be used for background tasks like connecting to wifi
  taskHandler.takeSemaphore("WiFi", portMAX_DELAY);
  if(!WiFi.isConnected()) { // If we are not connected to wifi and not already trying to connect
    int n = WiFi.scanNetworks();
    for(int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      if(preferences.isKey(ssid.c_str())) {
        String networkInfo;
        networkInfo = preferences.getString(ssid.c_str());
        log_d("Found known network: %s | %s", ssid.c_str(), networkInfo.c_str());
        switch(WiFi.encryptionType(i)) {
          case WIFI_AUTH_OPEN:
            WiFi.begin(ssid.c_str());
            break;
          case WIFI_AUTH_WEP:
          case WIFI_AUTH_WPA_PSK:
          case WIFI_AUTH_WPA2_PSK:
          case WIFI_AUTH_WPA_WPA2_PSK:
            {
              int delimiterIndex = networkInfo.indexOf('|');
              String password = networkInfo.substring(delimiterIndex + 1);
              log_d("Connecting to %s, %s|", ssid.c_str(), password.c_str());
              WiFi.begin(ssid.c_str(), password.c_str());
            }
            break;
          case WIFI_AUTH_ENTERPRISE:
            {
              int delimiter = networkInfo.indexOf('|');
              String username = networkInfo.substring(0, delimiter);
              String password = networkInfo.substring(delimiter + 1);
              log_d("Connecting to %s, %s, %s|", ssid.c_str(), username.c_str(), password.c_str());
              WiFi.begin(ssid.c_str(), WPA2_AUTH_PEAP, username.c_str(), username.c_str(), password.c_str());
            }
            break;
        // case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
        //     {
        //       int delimiter = networkInfo.indexOf('|');
        //       String username = networkInfo.substring(0, delimiter);
        //       String password = networkInfo.substring(delimiter + 1);
        //       log_d("Connecting to %s, %s, %s|", ssid.c_str(), username.c_str(), password.c_str());
        //       WiFi.begin(ssid.c_str(), WPA2_AUTH_TTLS, username.c_str(), username.c_str(), password.c_str());
        //     }
            break;
          default:
            log_w("Unsupported encryption type for SSID: %s", ssid.c_str());
            break;
        }
        log_d("Connecting to %s", ssid.c_str());
        uint8_t counter = 0;
        while (WiFi.status() != WL_CONNECTED) {
          delay(100);
          counter++;
          if (counter > 70) { // Timeout after 7 seconds
              Serial.println("Connection timed out");
              break;
          }
        }
        if(WiFi.status() == WL_CONNECTED) {
          log_i("Connected to %s, IP: %s", ssid.c_str(), WiFi.localIP().toString().c_str());
          WiFi.setAutoReconnect(true);
          break; // Exit the loop if connected
        } else {
          log_w("Failed to connect to %s", ssid.c_str());
        }
        taskHandler.releaseSemaphore("WiFi");
        delay(3000); // Short delay before trying the next network, allow WiFi app to run if needed
        taskHandler.takeSemaphore("WiFi");
      }
    }
  }
  taskHandler.releaseSemaphore("WiFi");
  delay(30000); // Wait 30 seconds before trying to connect to wifi
}