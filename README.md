# Smart Outdoor Security System

This guide will lead you through the steps to configure a camera client-server using the ESP32-WROVER module.

## Prerequisites

Before you begin, make sure you have the following:

- PlatformIO, Arduino
- ESP32 any module with CAMERA
- PIR MOTION SENSOR

## Steps

### 1. Camera PINs

1. Visit the repository [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32).

2. Search model camera PINs.

### 2. Setup wifi connection and config camera

The default configuration works for the ESP32WROVER module

- **Modify Camera PINs**: Replace camera pins on soss.h with the pins of  your ESP32 model.

- **Edit ssid and password values**: Go to main.cpp and replace values with your wifi connection.

### 3. Config server endpoint

On client_http_service.cpp search the url variable and replace it with your server endpoint.
