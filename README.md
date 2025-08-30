# STP Smartwatch Firmware

This repository contains the firmware for the STP smartwatch, designed for the ESP32-S3-Touch-LCD-1.28 development board. The firmware is written in C using the ESP-IDF framework and utilizes the LVGL graphics library to provide a responsive, touch-enabled user interface.

The watch features include basic timekeeping, fitness tracker, and a stopwatch. It runs on FreeRTOS for efficient multitasking and supports low-power sleep modes. The project aims to deliver a modular, easy-to-understand codebase that can be extended or adapted for personal use.

## Features

- **Real-Time Clock**: Displays the current time using the internal RTC or synchronized source.
- **Graphical UI**: Built using LVGL with support for smooth screen transitions and swiping between pages.
- **Step Counter**: Using the IMU it tracks the wearers steps.
- **Stopwatch**: Start, pause, and reset functionality with visual feedback.
- **Touch Input**: Navigate the UI using the capacitive touch screen with swipe gesture recognition.
- **FreeRTOS Integration**: All tasks are handled through a real-time operating system for concurrency and responsiveness.
- **Low Power Modes**: Screen timeout and sleep logic implemented to preserve battery life.
- **Modular Codebase**: Peripherals and UI components are modularized for easier extension and maintenance.

## Hardware Setup

The firmware is designed specifically for the [ESP32-S3-Touch-LCD-1.28 development board](https://www.espressif.com/en/products/devkits/esp32-s3-touch-lcd-128), which includes:

- An ESP32-S3 microcontroller with integrated Wi-Fi and Bluetooth.
- A 1.28-inch round GC9A01 LCD display with touch input.
- Onboard IMU sensor for motion and gesture detection.
- Backlight control via a dedicated GPIO pin.
- USB-C interface for power and programming.

All necessary GPIO pins and peripheral configurations for the display, touch controller, and IMU are defined in the source code, matching the board’s schematic.
## Getting Started

### Hardware Requirements

This firmware is designed specifically for the **ESP32-S3** microcontroller and tested on the [ESP32-S3 Dev Board](https://www.espressif.com/en/products/devkits/esp32-s3-devkitc-1). It relies on the following hardware components:

- ESP32-S3 chip  
- GC9A01 display controller with SPI interface  
- Compatible touch sensor hardware (as implemented in this project)  
- Onboard GPIO pin configuration matching the dev board  

**Important:** This project is *not* guaranteed to work on other ESP32 variants (such as ESP32, ESP32-C3, or ESP32-S2) or on boards with different peripherals or pin configurations without modification.

---
## Building from Source
COMING SOON

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

