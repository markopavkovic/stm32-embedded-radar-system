# stm32-embedded-radar-system
Non-blocking STM32 radar system with CAN bus telemetry, Processing GUI, and multi-mode keypad control.
---

## 🚀 Key Features & Modes
- **Mode A (Autonomous Sweep):** Continuous servo scanning ($0^\circ - 180^\circ$) with dynamic distance tracking.
- **Mode B (Step Control):** Precise manual stepping via $4 \times 4$ Matrix Keypad input.
- **Mode C (High-Speed GUI Telemetry):** Real-time polar-to-Cartesian data stream to Processing GUI.
- **Mode D (Manual Lock):** Direct coordinate locking from user keypad entry.

---

## 🛠️ Protocols & Hardware Peripherals
- **UART (115200 Baud):** Transmits angle/distance telemetry frames to the companion Processing 2D GUI.
- **SPI:** Interfaces with the **MCP2515 CAN Controller** for $500\text{ kbps}$ bus telemetry broadcast.
- **I²C:** Drives the **SSD1306 OLED** screen for real-time status and mode displays.
- **Actuation & Energy Saving:** **SG90 Micro Servo** driven via PWM with **MOSFET power gating** to eliminate idle current consumption.
- **Sensing:** **HC-SR04** ultrasonic sensor with non-blocking timing logic.

---

## 📂 Repository Structure
- `Firmware/` - STM32 C/C++ embedded source code, drivers, and state machine architecture.
- `GUI/` - Processing Java source code for the 2D visual radar scope.
- `Media/` - Project photos, schematics, and demo footage.

---
