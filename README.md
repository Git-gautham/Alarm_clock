# Alarm_clock
# ESP32 Dual TM1637 Multi-Mode Digital Clock

An advanced **multi-mode digital clock system** built using **ESP32**, **DS3231 RTC**, and **two TM1637 4-digit displays**.  
This project supports **9 functional modes**, including time/date display, alarm setting, stopwatch, timer, and manual time/date configuration — all controlled via just four tactile buttons.

---

## 🧩 Features

| Mode | Description |
|------|--------------|
| 1️⃣ | **HH:MM SS A/P** — 12-hour format time with AM/PM indicator |
| 2️⃣ | **HH:MM DDMM** — Displays hour, minute, and date |
| 3️⃣ | **DDMM YYYY** — Displays full date |
| 4️⃣ | **Alarm A Set** — Set first alarm (24-hour format) |
| 5️⃣ | **Alarm B Set** — Set second alarm (24-hour format) |
| 6️⃣ | **Stopwatch** — Start/Stop/Reset via buttons |
| 7️⃣ | **Timer** — Set countdown timer (HHMMSS) |
| 8️⃣ | **Time Set** — Sequential hour, minute, second entry |
| 9️⃣ | **Date Set** — Sequential day, month, year entry |

---

## 🖲️ Controls

| Button | GPIO | Function |
|---------|------|-----------|
| Mode | 23 | Cycle through modes |
| + (Increment) | 25 | Increase value / Start stopwatch |
| – (Decrement) | 26 | Decrease value / Stop stopwatch |
| Set | 27 | Confirm / Move to next digit / Reset stopwatch |

> ⚙️ While in any *set* mode, changing the mode mid-way aborts the current edit.

---

## 💡 Hardware Setup

### Components:
- **ESP32 Dev Module**
- **DS3231 RTC Module (I²C)**
- **2× TM1637 4-digit 7-segment displays**
- **4× tactile push buttons**

### Pin Connections:

| Component | ESP32 Pin | Notes |
|------------|------------|-------|
| TM1637 Display 1 CLK | 16 | Upper display |
| TM1637 Display 1 DIO | 17 | |
| TM1637 Display 2 CLK | 18 | Lower display |
| TM1637 Display 2 DIO | 19 | |
| DS3231 SDA | 21 | I²C data |
| DS3231 SCL | 22 | I²C clock |
| Mode Button | 23 | INPUT_PULLUP |
| INC (+) Button | 25 | INPUT_PULLUP |
| DEC (–) Button | 26 | INPUT_PULLUP |
| SET Button | 27 | INPUT_PULLUP |

---

## 🔧 Software Details

### Libraries Used
```cpp
#include <Wire.h>
#include "uRTCLib.h"
#include <TM1637Display.h>
