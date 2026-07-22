#  AeroCore – ESP32 Drone Altitude Monitoring flight-controller

AeroCore is an embedded systems project that simulates the core logic of a drone flight controller. 
It continuously measures altitude using an ultrasonic sensor, filters noisy sensor data, computes control corrections using a PID algorithm, and manages flight behavior through a Finite State Machine (FSM).
Real-time telemetry is displayed on an OLED screen and indicated through status LEDs.

> **Note:** This project simulates the decision-making logic of a drone flight controller. It does not control motors or fly a drone.

---

##  Features

-  Real-time altitude measurement using HC-SR04
-  Moving Average Filter for stable sensor readings
-  PID-based altitude control simulation
-  Finite State Machine (FSM)
  - IDLE
  - TAKEOFF
  - HOLD
  - LANDING
  - EMERGENCY
- Live telemetry on SSD1306 OLED Display
-  LED status indication
-  Serial Monitor commands
-  Serial Plotter compatible output
- ⚡ Built on ESP32 using Arduino Framework

---
# Flight States

## 🟢 IDLE
- System initialized
- Waiting for user command
- Green LED ON

## 🟡 TAKEOFF
- Attempts to reach target altitude
- PID controller active
- Yellow LED blinking

## 🟢 HOLD
- Target altitude achieved
- PID maintains altitude
- Green LED ON

## 🟡 LANDING
- Controlled descent
- Yellow LED ON

## 🔴 EMERGENCY
- Sensor failure detected
- Red LED blinking

# Hardware Used

| Component | Quantity |
|-----------|---------:|
| ESP32 Dev Board | 1 |
| HC-SR04 Ultrasonic Sensor | 1 |
| SSD1306 OLED Display (I2C) | 1 |
| LEDs (Green, Yellow, Red) | 3 |
| 220Ω Resistors | 3 |
| Breadboard | 1 |
| Jumper Wires | As Required |

---

# Circuit Connections

## HC-SR04

| Sensor | ESP32 |
|---------|--------|
| VCC | 5V |
| GND | GND |
| TRIG | GPIO 5 |
| ECHO | GPIO 18 *(through voltage divider)* |

---

## OLED Display

| OLED | ESP32 |
|------|-------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

---

## LEDs

| LED | GPIO |
|-----|------|
| Green | GPIO 25 |
| Yellow | GPIO 26 |
| Red | GPIO 27 |

Each LED is connected through a **220Ω resistor** to the corresponding GPIO pin.

---

# How It Works

1. The HC-SR04 continuously measures altitude.
2. A Moving Average Filter smooths noisy sensor readings.
3. The system compares the measured altitude with the target altitude.
4. The PID controller calculates the required correction.
5. The Finite State Machine manages the current flight mode.
6. Telemetry is displayed on the OLED.
7. LEDs indicate the current system state.
8. Data is streamed to the Serial Monitor and Serial Plotter.

---

# Serial Commands

| Command | Function |
|----------|----------|
| start | Begin takeoff sequence |
| land | Begin landing sequence |
| reset | Reset the system |

---

# LED Indicators

| Flight State | LED Status |
|--------------|-----------|
| IDLE | 🟢 Green ON |
| TAKEOFF | 🟡 Yellow Blinking |
| HOLD | 🟢 Green ON |
| LANDING | 🟡 Yellow ON |
| EMERGENCY | 🔴 Red Blinking |

---

# Software

- Arduino IDE
- ESP32 Arduino Core
- Adafruit GFX Library
- Adafruit SSD1306 Library

---

# Project Structure

```
Aerocore_ESP/
│
├── Aerocore_ESP.ino
├── images/
│   ├── testing.jpeg
│   └── messysetup.jpeg
└── README.md
```

---

# Future Improvements

- Bluetooth control
- Wi-Fi dashboard
- Battery monitoring
- Flight data logging
- EEPROM configuration storage
- Real quadcopter motor control

---

# Learning Outcomes

This project demonstrates practical implementation of:

- Embedded Systems Programming
- Sensor Interfacing
- Signal Filtering
- PID Control
- Finite State Machines
- Real-Time Telemetry
- Hardware Debugging
- ESP32 Development


