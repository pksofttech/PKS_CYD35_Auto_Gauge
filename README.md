# PKS AUTO-TECH: OBD2 Smart Gauge & 4x4 Inclinometer (Jimny JB74 Edition)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Build%20Passing-brightgreen.svg)](https://platformio.org/)
[![LVGL Version](https://img.shields.io/badge/LVGL-v9.5.0-blue.svg)](https://lvgl.io/)
[![LovyanGFX](https://img.shields.io/badge/LovyanGFX-v1.2.28-orange.svg)](https://github.com/lovyan03/LovyanGFX)
[![Hardware](https://img.shields.io/badge/Board-ESP32--3248S035R-red.svg)](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
[![C++ Standard](https://img.shields.io/badge/Standard-C%2B%2B17-blueviolet.svg)](https://isocpp.org/)

**PKS_CYD35_Auto_Gauge** เป็นโปรเจกต์พัฒนาระบบหน้าปัดอัจฉริยะ **Automotive OBD-II Smart Gauge HUD** และ **4x4 Off-Road Inclinometer (Suzuki Jimny JB74 Edition)** สำหรับบอร์ด **ESP32-3248S035R** (CYD 3.5" ST7796 480x320 พร้อม Resistive Touch) ขับเคลื่อนด้วยกราฟิกเอนจิน **LVGL v9** และไดรเวอร์จอความเร็วสูง **LovyanGFX**

---

## 📸 Screen Modes & Interface Layout

### 1. Mode 1: Cockpit HUD Dashboard (หน้าปัดความเร็วและสมรรถนะเครื่องยนต์)
หน้าปัดดิจิทัลความคมชัดสูงสไตล์รถสปอร์ต แบ่งสัดส่วนแบบ Responsive Layout (24% | 48% | 26%):

```
+-----------------------------------------------------------------------------------+
| [OK OBD-II]               PKS AUTO-TECH COCKPIT HUD                   [MODE: HUD] |
+------------------+------------------------------------+---------------------------+
|   TACHOMETER     |          SPEEDOMETER DIAL          |     DIGITAL TELEMETRY     |
|    (0-8k RPM)    |           (0-200 KM/H)             |                           |
|                  |                                    | ECT: 92 °C                |
|      _---_       |              _------_              | [==========-------] (Bar) |
|    /       \     |            /    ||    \            |                           |
|   |   3.2   |    |           |   --  --   |           | BOOST: 1.1 BAR            |
|    \  RPM  /     |           |     73     |           | [======-----------] (Bar) |
|      '---'       |           |    KM/H    |           |                           |
|                  |            \          /            | BATT: 14.1 V              |
|  Prog. Arc Neon  |              '------'              | [============-----] (Bar) |
|  Redline Alert   |   Outer Scale + Analog Sport Needle|                           |
|  (6,500+ RPM)    |   Progressive Neon Speed Arc + Hub | AFR: 14.7                 |
|                  |   Large 36pt Digital Readout       | [=========--------] (Bar) |
+------------------+------------------------------------+---------------------------+
|  [TINT] 7/8 Fuel |          PKS AUTO-TECH             |          SYS 14:32        |
+-----------------------------------------------------------------------------------+
```

* **Left Panel (24%)**: Progressive Tachometer Arc (0–8,000 RPM) แสดงค่ารอบเครื่องยนต์ พร้อมเปลี่ยนสีเป็นสีแดง Crimson เมื่อเข้าสู่โซน Redline (> 6,500 RPM)
* **Center Panel (48%)**: Analog Speedometer Dial (0–200 km/h) พร้อมสเกลขีดรอบวง เข็มไมล์แบบ Sport Needle, Progressive Speed Arc, และตัวเลขดิจิทัลขนาดใหญ่ 36pt
* **Right Panel (26%)**: Linear Progress Bars 4 ช่อง แสดงผลข้อมูลเซนเซอร์พร้อมระบบเตือนสีแบบ Real-time:
  * **ECT (Coolant Temp)**: 40–120 °C (เตือนสีแดงเมื่ออุณหภูมิ > 105 °C)
  * **BOOST / MAP**: 0.0–2.5 BAR
  * **BATTERY**: 10.0–16.0 V (เตือนสีส้มเมื่อแรงดัน < 11.8V หรือ > 14.8V)
  * **AFR (Air-Fuel Ratio)**: 10.0–20.0
* **Bottom Status Bar**: ระดับน้ำมันเชื้อเพลิง, ชื่อแบรนด์ PKS AUTO-TECH, และเวลาระบบ

---

### 2. Mode 2: 4x4 Off-Road Inclinometer (Suzuki Jimny JB74 Edition)
หน้าปัดวัดมุมเอียงรถออฟโรดแบบ Real-time พร้อมโมเดล Vector Silhouette หมุนและเอียงตามองศาจริง:

```
+-----------------------------------------------------------------------------------+
| [OK OBD-II]              4X4 OFF-ROAD INCLINOMETER                    [MODE: 4X4] |
+-----------------------------------------+-----------------------------------------+
|               ROLL ANGLE                |               PITCH ANGLE               |
|             (-45° to +45°)              |             (-45° to +45°)              |
|                                         |                                         |
|                 _---_                   |                 _---_                   |
|               /       \                 |               /       \                 |
|              |    _    |                |              |   __    |                |
|              |  [4x4]  | <-- Jimny หน้ารถ|              |  \4x4\  | <-- Jimny ตัวรถ |
|              |  ===|===|     เอียง ซ้าย/ขวา|             |  ==/=== |     เชิดขึ้น/ลง |
|               \       /                 |               \       /                 |
|                 R 18° >                 |                 UP +24°                 |
|               (STABLE)                  |               44.5% Grade               |
|                                         |                                         |
| * Symmetrical Bipolar Arc (-45°..+45°)  | * Symmetrical Bipolar Arc (-45°..+45°)  |
| * Suzuki Jimny Front (Grille + Lights)  | * Suzuki Jimny Side (Roof + Spare Tire) |
| * Rollover Hazard Warning (> 35°)       | * Real-time % Grade Calculation         |
+-----------------------------------------+-----------------------------------------+
| HDG: 270° W      |     ALT: 980 m       |     SPD: 45 KM/H   |     BAT: 14.1 V    |
+-----------------------------------------------------------------------------------+
```

* **Roll Dial (มุมเอียงซ้าย-ขวา)**:
  * โมเดล **Suzuki Jimny JB74 Front Silhouette**: กระจังหน้า 5 ช่องแนวตั้ง, ไฟหน้าทรงกลม, โป่งซุ้มล้อ และกระจกมองข้าง
  * หมุนเอียงตามมุม Roll แบบ Sub-pixel เทียบกับเส้นระดับขอบฟ้า (Horizon Reference Line)
  * ระบบเตือนความปลอดภัย: `STABLE` (ปกติ) $\rightarrow$ `CAUTION` ($\ge 20^\circ$) $\rightarrow$ `ROLLOVER DANGER` ($\ge 35^\circ$)
* **Pitch Dial (มุมไต่ทางชัน ขึ้น-ลง)**:
  * โมเดล **Suzuki Jimny JB74 Side Silhouette**: กระจกหน้าตั้งชัน, หลังคาทรงเหลี่ยม, ซุ้มล้อเหลี่ยม และ**ยางอะไหล่ห้อยท้ายฝากระโปรงหลัง**
  * ตัวรถเชิดหน้าขึ้นตามมุมไต่ชัน (`UP +xx°`) หรือทิ่มลงตามมุมลงเนิน (`DN xx°`)
  * คำนวณความชันถนนแบบ **% Grade** ($Grade\% = \tan(\theta) \times 100$)
* **Offroad Telemetry Footer**: เข็มทิศดิจิทัล (Compass Heading & Cardinal Direction), ความสูงจากระดับน้ำทะเล (Altitude Meter), ความเร็วรถ และแรงดันแบตเตอรี่

---

### 3. PKS Boot & Self-Test Screen
* หน้าจอเริ่มต้นระบบ Diagnostic Boot Screen สไตล์ยานยนต์ไฮเทค
* หลอดไฟ LED สถานะระบบ (ECU, SENSORS, TFT, TOUCH)
* แอนิเมชันแถบสถานะโหลดระบบ 0–100% พร้อมข้อความ System Ready ก่อนสลับเข้าหน้าปัดหลักแบบ Fade-In อัตโนมัติ

---

## 🛠️ ข้อมูลฮาร์ดแวร์ (Hardware Specifications)

| ส่วนประกอบ | รายละเอียด |
| :--- | :--- |
| **MCU** | ESP32-WROOM-32 (Dual-Core Tensilica LX6 @ 240MHz, 520KB SRAM, 4MB Flash) |
| **Display** | 3.5" TFT LCD ST7796S (ความละเอียด 480x320 พิกเซล, 65K Colors RGB565) |
| **Touchscreen** | XPT2046 4-Wire Resistive Touch (แชร์บัส HSPI ร่วมกับจอ) |
| **Display Bus** | HSPI Bus ความเร็ว 40MHz พร้อม DMA Channel Auto |
| **Backlight** | GPIO 27 (ควบคุมด้วย Hardware PWM ความถี่ 5kHz, Auto-Dimming & Smooth Fade) |
| **On-board RGB** | GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) - Active LOW |

### Pinout Mapping (ESP32-3248S035R)

```
+------------------+---------------+------------------------------------------+
| Function         | ESP32 Pin     | Description                              |
+------------------+---------------+------------------------------------------+
| SPI SCK (CLK)    | GPIO 14       | Shared HSPI Clock (TFT & Touch)          |
| SPI MOSI (SDI)   | GPIO 13       | Shared HSPI Data Out                     |
| SPI MISO (SDO)   | GPIO 12       | Shared HSPI Data In                      |
| TFT CS           | GPIO 15       | Display Chip Select                      |
| TFT DC (RS)      | GPIO 2        | Data / Command Select                    |
| TFT RST          | EN / Reset    | Reset ร่วมกับปุ่มบอร์ด                   |
| TFT Backlight    | GPIO 27       | PWM Brightness Control                   |
| Touch CS         | GPIO 33       | Touch Chip Select                        |
| RGB LED (Red)    | GPIO 4        | Active LOW Indicator                     |
| RGB LED (Green)  | GPIO 16       | Active LOW Indicator                     |
| RGB LED (Blue)   | GPIO 17       | Active LOW Indicator                     |
| I2C (Optional)   | GPIO 22 (SCL) | เชื่อมต่อ IMU Sensor (MPU6050/ICM20948)  |
|                  | GPIO 21 (SDA) |                                          |
| CAN / OBD-II     | GPIO 5 (TX)   | เชื่อมต่อ CAN Transceiver (SN65HVD230)   |
|                  | GPIO 35 (RX)  |                                          |
+------------------+---------------+------------------------------------------+
```

---

## 💻 โครงสร้างซอฟต์แวร์ (Software Architecture)

```
PKS_CYD35_Auto_Gauge/
├── include/
│   ├── LGFX_ESP32_3248S035.hpp   # ไดรเวอร์ LovyanGFX และ Pin Mapping ของจอ CYD 3.5"
│   ├── backlight_manager.hpp     # ระบบจัดการแสงหน้าจอ Auto-Dimming และ Fade PWM
│   ├── boot_screen.h             # เฮดเดอร์หน้าจอ Boot Diagnostic Screen
│   ├── demo_ui.hpp               # ฟังก์ชัน UI และ Touch Debug
│   ├── lv_conf.h                 # ไฟล์กำหนดค่า LVGL v9 (Fonts, Scales, Arcs, Lines)
│   └── obd_dashboard.h           # โครงสร้างข้อมูล Telemetry, API และ Thresholds
├── src/
│   ├── boot_screen.c             # การสร้างและทำงานของหน้า Boot Splash Screen
│   ├── main.cpp                  # จุดเริ่มต้นโปรแกรม (Setup, Loop, Drivers, Calibration)
│   └── obd_dashboard.c           # โค้ดหลัก HUD Cockpit, Jimny Inclinometer, Vector Math
├── platformio.ini                # การตั้งค่าบิลด์ PlatformIO (C++17, Libraries, Flags)
└── README.md                     # เอกสารแนะนำและคู่มือการใช้งานโปรเจกต์
```

### ไฮไลท์การออกแบบเชิงวิศวกรรม (Engineering Highlights)
1. **Zero Runtime Heap Allocations**: ฟังก์ชัน `obd_dashboard_update()` อัปเดตข้อมูลผ่านบัฟเฟอร์หน่วยความจำคงที่ ไม่มีการเรียก `malloc()` หรือจัดสรร Heap ขณะทำงาน จึงหมดปัญหา Memory Leak หรือ Memory Fragmentation
2. **2D Vector Trigonometric Transformation**: การหมุนโมเดลรถ Jimny และเข็มหน้าปัดคำนวณผ่าน Cosine/Sine Matrix ระดับ Sub-pixel ความเร็วสูง < 0.05ms
3. **NVS Touch Calibration**: บันทึกพิกัด Calibrate ของทัชสกรีนลง Flash Memory (ESP32 Preferences) โหลดใช้งานอัตโนมัติไม่ต้อง Calibrate ซ้ำทุกครั้งที่เปิดเครื่อง
4. **Auto-Dimming Backlight**: ระบบตรวจจับ Inactivity 60 วินาทีเพื่อหรี่แสงหน้าจออัตโนมัติ ช่วยลดความร้อนและยืดอายุหลอดไฟ LED ของจอ TFT

---

## 🚀 การติดตั้งและคอมไพล์ (Getting Started)

### ความต้องการของระบบ
* **VS Code** พร้อมติดตั้งส่วนขยาย **PlatformIO IDE** (หรือใช้ PlatformIO Core CLI)
* สาย Micro-USB สำหรับเชื่อมต่อบอร์ด ESP32 CYD 3.5" เข้ากับคอมพิวเตอร์

### 1. โคลนโปรเจกต์ (Clone Repository)
```bash
git clone https://github.com/pksofttech/PKS_CYD35_Auto_Gauge.git
cd PKS_CYD35_Auto_Gauge
```

### 2. คอมไพล์โปรแกรม (Build Firmware)
```bash
pio run
```

### 3. แฟลชโปรแกรมลงบอร์ด (Upload Firmware)
```bash
pio run -t upload
```

### 4. เปิดดู Serial Monitor
```bash
pio device monitor -b 115200
```

---

## 🔌 การต่อยอดฮาร์ดแวร์ภายนอก (Hardware Expansion)

### 1. เชื่อมต่อเซนเซอร์วัดมุมเอียง (IMU Sensor)
เชื่อมต่อโมดูล **MPU-6050** หรือ **ICM-20948** ผ่านบัส I2C:
* `VCC` $\rightarrow$ `3.3V`
* `GND` $\rightarrow$ `GND`
* `SCL` $\rightarrow$ `GPIO 22`
* `SDA` $\rightarrow$ `GPIO 21`

ส่งค่า Roll & Pitch เข้าฟังก์ชัน:
```c
obd2_telemetry_t data = *obd_dashboard_get_current_data();
data.roll_deg  = imu.getRoll();
data.pitch_deg = imu.getPitch();
obd_dashboard_update(&data);
```

### 2. เชื่อมต่อพอร์ต OBD-II ของรถยนต์ (CAN Bus Transceiver)
เชื่อมต่อโมดูล **SN65HVD230** (3.3V CAN Transceiver) เพื่อดึงข้อมูล PIDs ผ่านบัส ESP32 TWAI (CAN):
* `CTX (TX)` $\rightarrow$ `GPIO 5`
* `CRX (RX)` $\rightarrow$ `GPIO 35`
* `CAN_H / CAN_L` $\rightarrow$ ขา 6 และ 14 ของพอร์ต OBD-II รถยนต์

---

## 📄 License
โปรเจกต์นี้เผยแพร่ภายใต้ **MIT License** - สามารถนำไปศึกษา ปรับแต่ง และพัฒนาต่อยอดได้อย่างอิสระ

**PKS AUTO-TECH** | Developed with passion for Automotive Embedded Systems & GUI.
