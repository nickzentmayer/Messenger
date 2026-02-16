#include "GUI.h"


//Variables
int currentApp = 0;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite bkg = TFT_eSprite(&tft);

bool GUI::shift = false;
bool GUI::caps = false;
bool GUI::syms = false;
bool GUI::alt = false;
AppBase* GUI::apps[APP_COUNT + 1] = {nullptr};

void GUI::guiTask(void *pvParameters) {
     TaskHandler* taskHandler = (TaskHandler *)pvParameters;
    // Initialize the TFT display
    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(1);
    // Initialize apps
    apps[HOME] = new Home(taskHandler);
    apps[WIFI] = new WifiApp(taskHandler);
    apps[MESSAGES] = new MessagesApp(taskHandler);
    apps[DEBUG_APP] = new DebugApp(taskHandler);
    apps[KEYBOARD] = new HIDKeyboard(taskHandler);

    // Load the background
    taskHandler->takeSemaphore("fs", portMAX_DELAY);
    bkg.createSprite(tft.width(), tft.height());
    renderPNG("/assets/background.png").pushToSprite(&bkg, 0, 0);
    taskHandler->releaseSemaphore("fs");
    //start the home app
    apps[HOME]->start();

    // Main loop for GUI task
    while (true) {
        // Here you can add code to update the GUI, handle user input, etc.
        while (Keyboard::keybInput != nullptr) {
            while(!taskHandler->takeSemaphore("keyboardRW", portMAX_DELAY));
            KeyQueue* current = Keyboard::keybInput; // Get the first key in the queue
            Keyboard::keybInput = Keyboard::keybInput->next; // Move to the next key in the queue
            //this next 3 lines are cooked, only accept multiple key presses if they are the same key, or we discard the rest for now
            // if(Keyboard::keybInput->next != nullptr)
            //     Keyboard::keybInput = (Keymap[Keyboard::keybInput->next->col][Keyboard::keybInput->next->row] == Keymap[current->col][current->row]) ? Keyboard::keybInput->next : nullptr; // Move to the next key in the queue
            // else Keyboard::keybInput = nullptr; // No more keys in the queue
            taskHandler->releaseSemaphore("keyboardRW");
            // Process the key event
            if(Keymap[current->col][current->row] == '^') shift = current->pressed;
            if(Keymap[current->col][current->row] == '@') syms = current->pressed;
            if(Keymap[current->col][current->row] == '&') alt = current->pressed;

            if(syms) processKey(symKeymap[current->col][current->row], current->pressed);
            else if(caps ^ shift) processKey(capKeymap[current->col][current->row], current->pressed);
            else processKey(Keymap[current->col][current->row], current->pressed);

            delete current; // Free the memory for the processed key
        }
        processLoop();
        vTaskDelay(1 / portTICK_PERIOD_MS); // Delay to prevent busy-waiting
    }
}

void GUI::processKey(char key, bool pressed) {
    if(alt && key == ' ' && pressed) {
        apps[currentApp]->end(); // End the current app
        currentApp = HOME;
        apps[currentApp]->start(); // Start the home app
        return;
    }
    if(currentApp == HOME)  {
        int lastApp = currentApp;
        apps[currentApp]->update(key, pressed, alt);
        if(currentApp != lastApp) {
            apps[lastApp]->end(); // End the previous app
            apps[currentApp]->start(); // Start the new app
        }
    }
    else {
        apps[currentApp]->update(key, pressed, alt);
    }
}

void GUI::processLoop() {
    // Process the current app
    apps[currentApp]->run();
}