# ESP32 Digital Clock & Environmental Monitor

## Project Overview
This project is a high-precision digital clock and environmental monitor powered by the ESP32 microcontroller. It showcases professional-grade hardware design and firmware integration, focusing on scalability, safety, and manufacturing efficiency.

## Hardware Engineering
### Hierarchical Schematic Design
To ensure a clean and scalable architecture, the system was designed using a **Hierarchical Approach** in KiCad. 
* **Logical Partitioning:** The project is divided into functional blocks (MCU, Power Management, Display Driver, and Sensors).
* **Hierarchical Labels:** Connectivity between sheets is managed through hierarchical labels, significantly improving schematic readability and reducing wiring complexity.
* **Net Classes:** Custom net classes were defined to enforce specific routing rules. This ensures that power traces have the necessary width for current demands while sensitive signals, such as the I2C bus for the AHT10 and DS3231, maintain optimal spacing and integrity.

### Modular Display Stage
* **Shift Register Control:** Individual segments are managed via shift registers to minimize GPIO footprint on the ESP32.
* **Multi-channel Design:** Utilizing KiCad's multi-channel capabilities, the segment driver block was instanced for each digit, ensuring uniform performance and routing.
* **Hardware Versatility:** The PCB includes solder jumpers that allow the system to natively support both **Common Cathode** and **Common Anode** displays. This modularity enables assembly flexibility without requiring different board revisions.

## PCB Layout & Manufacturing
### Signal Integrity & Placement
* **Optimized Routing:** Communication traces were kept as short as possible to minimize noise and parasitic effects.
* **Space Optimization:** A "Bottom-Heavy" SMD placement strategy was used, keeping most components on the bottom layer to allow for a compact form factor and a clean top-side display interface.

### High-Voltage Safety (110V/100V)
* **Isolation Zones:** A significant physical clearance was maintained between the 110V AC input and the low-voltage DC logic.
* **No Ground Plane in HV Area:** To prevent arcing and electromagnetic interference (EMI) coupling into the logic circuits, the ground plane was intentionally removed from the high-voltage section.

## Video Demonstration
Click the link below for a technical walkthrough of the hardware architecture, schematic design, and PCB layout:

[📺 Watch the Technical Breakdown on YouTube](INSERT_YOUR_VIDEO_LINK_HERE)

---
*Developed by Julián Junior Torres Maldonado - Mechatronics Engineer*
