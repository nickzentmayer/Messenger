#pragma once
#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <Arduino.h>
#include <Adafruit_TCA8418.h>
#include "TaskHandler.h"

#define ROWS 7
#define COLS 5

const char keymap[COLS][ROWS] = {
    {'q', 'w', '@', 'a', '&', ' ', '+'},
    {'e', 's', 'd', 'p', 'x', 'z', '^'},
    {'r', 'g', 't', '^', 'v', 'c', 'f'},
    {'u', 'h', 'y', '\n', 'b', 'n', 'j'},
    {'o', 'l', 'i', '<', '$', 'm', 'k'}
};
const char symKeymap[COLS][ROWS] = {
    {'#', '1', 0, '*', '|', ' ', '0'},
    {'2', '4', '5', '@', '8', '7', '^'},
    {'3', '/', '(', '^', '?', '9', '6'},
    {'_', ':', ')', '\n', '!', ',', ';'},
    {'+', '"', '-', '<', '=', '.', '\''}
};
const char capKeymap[COLS][ROWS] = {
    {'Q', 'W', '@', 'A', '&', ' ', '\\'},
    {'E', 'S', 'D', 'P', 'X', 'Z', '^'},
    {'R', 'G', 'T', '^', 'V', 'C', 'F'},
    {'U', 'H', 'Y', '\n', 'B', 'N', 'J'},
    {'O', 'L', 'I', '<', '$', 'M', 'K'}
};

struct KeyQueue {
    uint8_t row;
    uint8_t col;
    bool pressed;
    KeyQueue* next = nullptr;
};

class Keyboard {
    public:
    static KeyQueue* keybInput;
    static void keyboardTask(void *pvParameters);
    private:
    static Adafruit_TCA8418 kb;
    static TaskHandler* taskHandler;
};

#endif // KEYBOARD_H