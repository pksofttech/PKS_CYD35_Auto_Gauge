/**
 * @file LGFX_ESP32_3248S035.hpp
 * @brief LovyanGFX Configuration for ESP32-3248S035R (CYD 3.5" Resistive Touch)
 * 
 * Hardware Specs:
 * - Display Controller: ST7796 (320x480)
 * - Touch Controller: XPT2046 (Resistive)
 * - Shared SPI Bus (SCLK: 14, MOSI: 13, MISO: 12)
 * - Display CS: 15, DC: 2, RST: -1, Backlight: 27
 * - Touch CS: 33, Touch IRQ: -1 (Polling mode)
 */

#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7796  _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX(void) {
        // ==========================================
        // 1. SPI Bus Configuration
        // ==========================================
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = HSPI_HOST;         // ใช้ HSPI (SPI2) บน ESP32 (Native Pin: 12, 13, 14, 15)
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;        // ความถี่ส่งข้อมูลจอ 40MHz
            cfg.freq_read  = 16000000;
            cfg.spi_3wire  = false;
            cfg.use_lock   = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;

            // Pinout ของจอ CYD 3.5"
            cfg.pin_sclk = 14;                // HSPI SCK
            cfg.pin_mosi = 13;                // HSPI MOSI
            cfg.pin_miso = 12;                // HSPI MISO
            cfg.pin_dc   = 2;                 // Data/Command (RS/DC)

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // ==========================================
        // 2. Display Panel (ST7796) Configuration
        // ==========================================
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 15;        // TFT CS
            cfg.pin_rst          = -1;        // TFT Reset (-1: ต่อร่วมกับ EN/RST ของบอร์ด)
            cfg.pin_busy         = -1;
            cfg.panel_width      = 320;       // ความกว้างจริงตามแนวตั้ง
            cfg.panel_height     = 480;       // ความสูงจริงตามแนวตั้ง
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = false;
            cfg.invert           = false;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = true;      // ใช้ SPI ร่วมกับ Touch

            _panel_instance.config(cfg);
        }

        // ==========================================
        // 3. Backlight (PWM) Configuration
        // ==========================================
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = 27;             // Backlight Pin (GPIO 27)
            cfg.invert      = false;
            cfg.freq        = 2000;           // ความถี่ PWM (2 kHz)
            cfg.pwm_channel = 7;

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // ==========================================
        // 4. Touch Controller (XPT2046) Configuration
        // ==========================================
        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 200;             // ค่า Raw ADC ต่ำสุดของ XPT2046 (Resistive)
            cfg.x_max      = 3800;            // ค่า Raw ADC สูงสุดของ XPT2046
            cfg.y_min      = 200;
            cfg.y_max      = 3800;
            cfg.pin_cs     = 33;              // Touch CS (GPIO 33)
            cfg.pin_int    = -1;              // ไม่มี T_IRQ (ใช้ Polling Mode)
            cfg.bus_shared = true;            // แชร์บัส SPI เดียวกับจอ
            cfg.spi_host   = HSPI_HOST;       // ใช้ HSPI บัสเดียวกับจอ
            cfg.freq       = 2500000;         // 2.5 MHz สำหรับอ่าน Touch

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};
