#include "Keyboard.h"
//KeyQueue* keybInput = nullptr;

KeyQueue* Keyboard::keybInput = nullptr;
Adafruit_TCA8418 Keyboard::kb = Adafruit_TCA8418();
TaskHandler* Keyboard::taskHandler = nullptr;

void Keyboard::keyboardTask(void *pvParameters) {
    taskHandler = (TaskHandler *)pvParameters;
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Allow time for the system to stabilize
    // Initialize the TCA8418 keyboard controller
    taskHandler->takeSemaphore("i2c", portMAX_DELAY);
    Wire.setPins(1, 2);
    while (!kb.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
        log_e("TCA8418 not found. Retrying...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    taskHandler->releaseSemaphore("i2c");
    log_d("Keyboard initialized successfully");
    // Set up the TCA8418
    kb.enableDebounce();
    kb.matrix(ROWS, COLS);
    kb.flush(); // Clear the internal buffer
    for(;;) {
        // Read the keys
        //taskHandler->takeSemaphore("i2c", portMAX_DELAY);
        while (kb.available() > 0) {
            int k = kb.getEvent();
            //find which key was pressed or released
            bool pressed = k & 0x80;
            k &= 0x7F;
            k--;
            uint8_t row = k / 10;
            uint8_t col = k % 10;
            KeyQueue* newKey = new KeyQueue;
            newKey->row = row;
            newKey->col = col;
            newKey->pressed = pressed;
            log_d("Key event detected: ['%c']", keymap[col][row]);
            while(!taskHandler->takeSemaphore("keyboardRW", portMAX_DELAY)) yield();
            if(keybInput == nullptr) keybInput = newKey;
            else {
                KeyQueue* current = keybInput;
                while(current->next != nullptr) current = current->next; // Traverse to the end of the list
                current->next = newKey;
            }
            taskHandler->releaseSemaphore("keyboardRW");
        }
    // taskHandler->releaseSemaphore("i2c");
    // vTaskDelay(10 / portTICK_PERIOD_MS); // Polling delay
    }
}