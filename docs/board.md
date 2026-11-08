# Waveshare ESP32-C6-LCD-1.3

Main page: https://docs.waveshare.com/ESP32-C6-LCD-1.3

## Specs (from the Waveshare docs)
| Item | Value |
|------|-------|
| MCU | ESP32-C6FH4, RISC-V 32-bit single core, up to 160 MHz |
| Memory | 320 KB SRAM, 4 MB stacked flash. PSRAM is not listed. |
| Wireless | 2.4 GHz Wi-Fi (802.11 b/g/n), Bluetooth 5 LE, onboard antenna |
| LCD | 1.3 inch IPS, 240 x 240, ST7789V2, 4-wire SPI |
| Other | RGB LED (WS2812B), TF card slot, BOOT and RESET buttons, 9-pin 2.54 mm header |
| USB | Type-C, used for flashing and serial logs |

## Pin map
The Waveshare pages show the pinout only as an image. These values come from `Display_ST7789.h`, `SD_Card.h` and `LVGL_Example.h` in the Arduino `LVGL_Arduino` demo in Waveshare's demo package. They have not been tested on the board yet.

| Function | GPIO |
|----------|------|
| SPI MISO | 5 |
| SPI MOSI | 6 |
| SPI SCLK | 7 |
| LCD CS | 14 |
| LCD DC | 15 |
| LCD RST | 21 |
| LCD backlight (PWM) | 22 |
| SD CS | 4 |
| WS2812B RGB LED | 8 |
| Button (`BUTTON_PIN` in the demo) | 9 |

## LCD settings used by the demo
- SPI clock: 80 MHz.
- Offsets: X 0, Y 0.
- Backlight: LEDC PWM at 1 kHz, 10-bit.
- The LCD and SD card share the SPI bus.

## Development
- Arduino: board package "ESP32 by Espressif Systems" 3.0.1. Waveshare's tested libraries include Arduino_GFX 1.4.9, ArduinoJson 6.21.2 and lvgl 8.3.10. LVGL v8 and v9 are not compatible.
- ESP-IDF: V5.5.0 or later is recommended.
- Download mode: hold BOOT while powering on.

## Links
- Schematic (saved here as `ESP32C6-1.3-schematic.pdf`): https://files.waveshare.com/wiki/ESP32-C6-LCD-1.3/ESP32C6-1.3.pdf
- Demo package: https://files.waveshare.com/wiki/ESP32-C6-LCD-1.3/ESP32-C6-LCD-1.3_Demo.zip
- Arduino guide: https://docs.waveshare.com/ESP32-C6-LCD-1.3/Arduino
- ESP-IDF guide: https://docs.waveshare.com/ESP32-C6-LCD-1.3/ESP-IDF
- Flashing guide: https://docs.waveshare.com/ESP32-C6-LCD-1.3/Firmware-Flashing
- ESP32-C6 datasheet: https://documentation.espressif.com/esp32-c6_datasheet_en.pdf
