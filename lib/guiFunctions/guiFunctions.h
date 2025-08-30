#pragma once
#ifndef GUI_FUNCTIONS_H
#define GUI_FUNCTIONS_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <FS.h>
#include <PNGdec.h>
#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "power.h"
#include <time.h>

#define FONT_SMALL NotoSansBold15
#define FONT_LARGE NotoSansBold36

extern TFT_eSPI tft;
//text
void centerText(String text, int* x, int* y);
//images
void * pngOpen(const char *filename, int32_t *size);
void pngClose(void *handle);
int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length);
int32_t pngSeek(PNGFILE *page, int32_t position);
TFT_eSprite renderPNG(String location);
TFT_eSprite renderStatusBar();

#endif