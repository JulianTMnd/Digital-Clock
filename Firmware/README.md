# Firmware Architecture & Implementation

This directory contains the C source code for the ESP32 digital clock. The firmware was developed natively using the **ESP-IDF** framework and is built around **FreeRTOS** to ensure modularity, and real-time execution.

## Core Features & Engineering Decisions

### Concurrent I2C Bus Management

The system relies on a shared I2C bus to communicate with both the **DS3231 Real-Time Clock (RTC)** and the **AHT10 Temperature/Humidity sensor**.

### Software Debouncing

Physical buttons and switches inherently suffer from mechanical bouncing, which can trigger multiple false inputs.

### Task Modularity

To maintain a clean codebase, the firmware avoids a bloated `main.c` file. The application is divided into independent tasks managed by the RTOS scheduler:

## Build & Flash Instructions

This project requires **ESP-IDF** (tested on v5.5.1). To compile and upload the firmware to your board, navigate to this directory in your terminal and use the standard IDF toolchain:

```bash
# 1. Set your target (if not already set)
idf.py set-target esp32c3

# 2. Build the project
idf.py build

# 3. Flash the firmware and open the serial monitor
idf.py -p (YOUR_PORT) flash monitor
```
