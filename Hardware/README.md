# ESP32 Digital Clock & Environmental Monitor

## Video Demonstration
Click the link below for a technical walkthrough of the hardware architecture, schematic design, and PCB layout:

[📺 Watch the Technical Breakdown on YouTube](https://youtu.be/VJSyPONk_Bk)


## Project Overview
This project is a high-precision digital clock and environmental monitor powered by the ESP32 microcontroller. It showcases professional-grade hardware design and firmware integration, focusing on scalability, safety, and manufacturing efficiency.

<img width="3840" height="2160" alt="Clock_02" src="https://github.com/user-attachments/assets/f720be08-d9a5-44eb-9a88-c9621108551c" />


## Hardware Engineering
### Hierarchical Schematic Design
To ensure a clean and scalable architecture, the system was designed using a **Hierarchical Approach** in KiCad. 
* **Logical Partitioning:** The project is divided into functional blocks (MCU, Power Management, Display Driver, and Sensors).
* **Hierarchical Labels:** Connectivity between sheets is managed through hierarchical labels, significantly improving schematic readability and reducing wiring complexity.
* **Net Classes:** Custom net classes were defined to enforce specific routing rules. This ensures that power traces have the necessary width for current demands while sensitive signals, such as the I2C bus for the AHT10 and DS3231, maintain optimal spacing and integrity.

<img width="1490" height="877" alt="KicadSch" src="https://github.com/user-attachments/assets/dd11c931-96bc-48e2-83d7-5a240532bba7" />




### Modular Display Stage
* **Shift Register Control:** Individual segments are managed via shift registers to minimize GPIO footprint on the ESP32.
* **Multi-channel Design:** Utilizing KiCad's multi-channel capabilities, the segment driver block was instanced for each digit, ensuring uniform performance and routing.
* **Hardware Versatility:** The PCB includes solder jumpers that allow the system to natively support both **Common Cathode** and **Common Anode** displays. This modularity enables assembly flexibility without requiring different board revisions.


<img width="859" height="726" alt="segments" src="https://github.com/user-attachments/assets/7ca06727-0e11-47ce-bc7e-ac412642f9c0" />



## PCB Layout & Manufacturing
### Signal Integrity & Placement
* **Optimized Routing:** Communication traces were kept as short as possible to minimize noise and parasitic effects.
* **Space Optimization:** A "Bottom-Heavy" SMD placement strategy was used, keeping most components on the bottom layer to allow for a compact form factor and a clean top-side display interface.

### High-Voltage Safety (110V)
* **Isolation Zones:** A significant physical clearance was maintained between the 110V AC input and the low-voltage DC logic.
* **No Ground Plane in HV Area:** To prevent arcing and electromagnetic interference (EMI) coupling into the logic circuits, the ground plane was intentionally removed from the high-voltage section.

<img width="765" height="601" alt="pcb_layout" src="https://github.com/user-attachments/assets/85ab183c-e2d0-49b9-8e7a-8f78f3efcae0" />


---
*Developed by Julián Junior Torres Maldonado - Mechatronics Engineer*
