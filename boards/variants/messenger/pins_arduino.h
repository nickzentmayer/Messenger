#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

#define USB_VID 0x303a
#define USB_PID 0x1001
//You can use the boot button as a normal button
static const uint8_t BUTTON_BOOT = 0;
//Default UART
static const uint8_t TX = 43;
static const uint8_t RX = 44;
//IMU I2C
static const uint8_t SDA = 1;
static const uint8_t SCL = 2;
//SPI - LoRa
static const uint8_t SCK  = 11;
static const uint8_t MISO = 13;
static const uint8_t MOSI = 12;
static const uint8_t SS   = 4;
//LoRa
static const uint8_t LORA_IRQ = 16;
static const uint8_t LORA_RST   = 10;
static const uint8_t LORA_BUSY = 17;
//Power
#define TFT_BACKLIGHT  GPIO_NUM_38
#define LDO_EN  GPIO_NUM_14
#define KEY_BACKLIGHT  GPIO_NUM_21

//Other ESP Stuff
// P1
static const uint8_t PIN_43 = 43;
static const uint8_t PIN_44 = 44;
static const uint8_t PIN_18 = 18;
static const uint8_t PIN_17 = 17;
static const uint8_t PIN_21 = 21;
static const uint8_t PIN_16 = 16;

// P2
static const uint8_t PIN_1  = 1;
static const uint8_t PIN_2  = 2;
static const uint8_t PIN_3  = 3;
static const uint8_t PIN_10 = 10;
static const uint8_t PIN_11 = 11;
static const uint8_t PIN_12 = 12;
static const uint8_t PIN_13 = 13;

// Analog
static const uint8_t A0  = 1;
static const uint8_t A1  = 2;
static const uint8_t A2  = 3;
static const uint8_t A9  = 10;
static const uint8_t A10 = 11;
static const uint8_t A11 = 12;
static const uint8_t A12 = 13;
static const uint8_t A15 = 16;
static const uint8_t A16 = 17;
static const uint8_t A17 = 18;


// Touch
static const uint8_t T1  = 1;
static const uint8_t T2  = 2;
static const uint8_t T3  = 3;
static const uint8_t T10 = 10;
static const uint8_t T11 = 11;
static const uint8_t T12 = 12;
static const uint8_t T13 = 13;

#endif /* Pins_Arduino_h */
