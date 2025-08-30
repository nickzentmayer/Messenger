#include "guiFunctions.h"

PNG pngDecoder;
using fs::File;

File pngfile;

void centerText(String text, int* x, int* y) {
    int width = tft.textWidth(text);
    if(x != nullptr) *x = (tft.width() - width) / 2;
    if(y != nullptr) *y = (tft.height() - tft.fontHeight()) /2;
}

TFT_eSprite png = TFT_eSprite(&tft);
#define MAX_IMAGE_WIDTH 320
int16_t xpos = 0;
int16_t ypos = 0;

void * pngOpen(const char *filename, int32_t *size) {
  //Serial.printf("Attempting to open %s\n", filename);
  pngfile = LittleFS.open(filename, "r");
  *size = pngfile.size();
  return &pngfile;
}

void pngClose(void *handle) {
  File pngfile = *((File*)handle);
  if (pngfile) pngfile.close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.seek(position);
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  pngDecoder.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  png.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

TFT_eSprite renderPNG(String location) {
  TFT_eSprite img = TFT_eSprite(&tft);
  int16_t rc = pngDecoder.open(location.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw);
      if (rc == PNG_SUCCESS) {
        img.createSprite(pngDecoder.getWidth(), pngDecoder.getHeight());
        png.createSprite(pngDecoder.getWidth(), pngDecoder.getHeight());
        //img.startWrite();
        //Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
        uint32_t dt = millis();
        
          rc = pngDecoder.decode(NULL, 0);
          pngDecoder.close();
        
        //img.endWrite();
        png.pushToSprite(&img, 0, 0, TFT_BLACK);
        png.deleteSprite();
        // How long did rendering take...
        //Serial.print(millis()-dt); //Serial.println("ms");
      }
      else {
        img.createSprite(60, 60);
        img.fillSprite(TFT_MAGENTA);
        img.loadFont(FONT_SMALL);
        img.setTextColor(TFT_BLACK, TFT_MAGENTA);
        img.drawString("Error loading PNG", 0, 0);
      }
  return img;
}

TFT_eSprite renderStatusBar() {
    TFT_eSprite statusBar = TFT_eSprite(&tft);
    statusBar.createSprite(tft.width(), 15);
    statusBar.fillSprite(TFT_BLACK);
    statusBar.loadFont(FONT_SMALL);
    statusBar.setTextColor(TFT_WHITE, TFT_BLACK);
    //while(!semHandler->takeSemaphore("status", portMAX_DELAY));
    struct tm tmstruct ;
    getLocalTime(&tmstruct, 0);
    String timeString = "";
    timeString += (tmstruct.tm_hour > 12) ? String(tmstruct.tm_hour - 12) + ":" : String(tmstruct.tm_hour) + ":";
    timeString += (tmstruct.tm_min < 10) ? "0" + String(tmstruct.tm_min) : String(tmstruct.tm_min);
    timeString += (tmstruct.tm_hour >= 12) ? " PM" : " AM";
    int centerX;
    centerText(timeString, &centerX, nullptr);
    statusBar.drawString(timeString, centerX, 0); //0 pixel padding from top
    // Draw battery status
    statusBar.fillRoundRect(statusBar.width() - 7, 4, 5, 7, 2, TFT_WHITE);
    statusBar.drawRoundRect(statusBar.width() - 40, 0, 35, statusBar.height(), 3, TFT_WHITE);
    statusBar.fillRoundRect(statusBar.width() - 39, 1, 33, statusBar.height() - 2, 2, TFT_BLACK);
    statusBar.fillRoundRect(statusBar.width() - 39, 1, map(batteryPercent, 0, 100, 0, 33), statusBar.height() - 2, 2, (batteryPercent < 20) ? TFT_RED : TFT_GREEN);

    statusBar.drawString(String((int)batteryPercent) + "%", (statusBar.width() - 45) - statusBar.textWidth(String((int)batteryPercent) + "%"), 0);

    //semHandler->releaseSemaphore("status");
    statusBar.unloadFont();
    return statusBar;
}