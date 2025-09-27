## STP Smartwatch Firmware

The STP Smartwatch is a compact, wearable device built on the **ESP32-S3-Touch-LCD-1.28 development board**. This simple smartwatch firmware provides a fully functional touchscreen interface, step tracking, a stopwatch, and battery monitoring. It is designed with modular, readable code, allowing users to easily adapt it as firmware for their own boards or reuse portions of the code for personal projects and experimentation.
---
## 🚧 Version 2 – Coming Soon 🚧

- **Custom PCB & Enhanced Sensors** – High-accuracy heart rate, SpO₂, temperature, and advanced motion tracking for precise activity monitoring.  
- **Advanced Health & Activity Tracking** – Heart rate monitor, improved step counter, and gesture detection.  
- **Optimized Power Management** – Efficient firmware and dynamic sensor control for longer battery life.  
- **Watch Face & Theme** – Improved UI theme that keeps energy consumption low(Potentially customizable UIs).  
- **Mobile App Integration** – Sync data, configure settings, and receive notifications directly from your smartphone.
  
---
## Version 1 Features

### Real-Time Clock
Displays the current time on your watch using either the internal clock or synchronized network time. Always shows the correct hour and minute in a simple, readable format.

### Graphical User Interface
Smooth, visually appealing screens built with LVGL. Navigate between different “tiles” or pages using swipe gestures on the touchscreen.

### Touch Input
Capacitive touch allows easy interaction. Swipe to switch screens and interact with watch features without buttons.

### Step Counter
Counts your steps using the built-in accelerometer/gyroscope. Tracks motion when worn on your wrist to estimate daily activity.

### Stopwatch
Simple start, pause, and reset functionality. Provides visual feedback directly on the watch screen.

### Battery Percentage Display
Shows the battery level on all screens in a consistent position above the main content. Updates in real-time so you know when to recharge.

### Modular Codebase
Drivers and UI components are separated to make the code easier to understand, maintain, and extend.

---

## Hardware Requirements

- **ESP32-S3 microcontroller** with integrated Wi-Fi/Bluetooth  
- **1.28-inch round GC9A01 LCD display**   
- **CST816S capacitive touch controller**  
- **Onboard IMU sensor** for motion detection and step tracking  
- USB-C connection for power, charging, and flashing  

> ⚠️ This firmware was designed specifically for the ESP32-S3-Touch-LCD-1.28 development board and may not work on other ESP32 variants without modification.

---

## Project Structure

```
C:.
│   CMakeLists.txt                    # Builds the firmware and sets project configuration
│   idf_component.yml                 # ESP-IDF metadata describing this project as a component
│   main.c                           # Entry point for firmware; sets up peripherals, UI, and main loop
│   project_tree.txt                 # Text version of the project tree (for reference)
│
├── connectivity
│   ├── time_sync.c / .h             # Handles SNTP/RTC time synchronization
│   ├── wifi.c / .h                  # Connects the board to Wi-Fi networks
│   └── wifi_config_local.h          # Local Wi-Fi credentials and configuration
│
├── drivers
│   ├── i2c_scan.c / .h              # Utility to detect devices on the I2C bus
│   ├── imu_driver.c / .h            # Interfaces with IMU to read motion data
│   ├── lcd_driver.c / .h            # Drives the GC9A01 display (initialization & updates)
│   ├── lvgl_tick.c / .h             # Provides millisecond tick for LVGL timing
│   └── touch_driver.c / .h          # Reads touch input from the CST816S controller
│
├── features
│   ├── battery_percentage.c / .h    # Reads ADC to show battery charge level
│   ├── step_counter.c / .h          # Calculates and tracks steps using IMU data
│   └── stopwatch.c / .h             # Implements stopwatch functionality (start/pause/reset)
│
├── fonts
│   ├── fonts.h                      # Declares font resources
│   ├── inter_14.c                   # 14px font used in the UI
│   ├── inter_20.c                   # 20px font used in the UI
│   ├── inter_32.c                   # 32px font used in the UI
│   └── inter_48.c                   # 48px font used in the UI
│
├── old_ui
│   ├── labels.c / .h                # Legacy UI labels used in early testing
│   └── screen.c / .h                # Legacy screen layout; did not support swipe gestures
│
└── ui
    ├── ui.c / .h                    # Current LVGL-based UI system with tiles, swipe, and battery display
```

---

## Flashing Instructions

1. Install the [ESP-IDF framework](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html) on your computer.  
2. Connect your ESP32-S3 board via USB-C.  
3. Open a terminal in the firmware project directory.  
4. Configure the project (adjust serial port if necessary):  
   ```bash
   idf.py set-target esp32s3
   idf.py menuconfig
5. Build the firmware:
   ```bash
   idf.py build
6. Flash the firmware to the board:
   ```bash
   idf.py flash
7. Monitor the output to ensure it boots correctly:
   ```bash
   idf.py monitor
8. After flashing, the watch should start displaing the default UI

---

## Demo

Insert the demo video of it working here

---

## License
 This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

 ---
