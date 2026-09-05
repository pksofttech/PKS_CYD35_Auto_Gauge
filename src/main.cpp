/**
 * @file main.cpp
 * @brief Base Starter Template for ESP32-3248S035R (CYD 3.5" ST7796 + XPT2046 Touch)
 * 
 * Hardware Specs:
 * - MCU: ESP32-WROOM-32 (240MHz, 520KB SRAM, 4MB Flash)
 * - Display: 3.5" TFT ST7796 (480x320 Landscape, HSPI)
 * - Touch: XPT2046 Resistive Touch (Shared HSPI)
 * - Backlight: GPIO 27 (PWM Controlled with Auto Dimming)
 * - RGB LED: GPIO 4 (Red), 16 (Green), 17 (Blue) - Active LOW
 * 
 * Libraries:
 * - LVGL v9.x (Light and Versatile Graphics Library)
 * - LovyanGFX v1.x (High-Performance Display & Touch Driver)
 */

#include <Arduino.h>
#include <Preferences.h>
#include <lvgl.h>
#include "LGFX_ESP32_3248S035.hpp"
#include "backlight_manager.hpp"
#include "demo_ui.hpp"
#include "obd_dashboard.h"
#include "boot_screen.h"

// =========================================================================
// Global Display & Touch Driver Instance
// =========================================================================
static LGFX lcd;

// ความละเอียดหน้าจอในโหมด Landscape (แนวนอน)
static const uint32_t screenWidth  = 480;
static const uint32_t screenHeight = 320;

// บัฟเฟอร์สำหรับเรนเดอร์กราฟิก LVGL v9 (1/10 ของขนาดหน้าจอ = 30.7 KB)
#define DRAW_BUF_SIZE (screenWidth * 32 * sizeof(lv_color16_t))
static uint8_t draw_buf[DRAW_BUF_SIZE];

// ตัวแปรและ Preferences สำหรับ Touch Calibration
static Preferences prefs;
bool calibrate_requested = false;

/* =========================================================================
 * 1. LVGL Display Flush Callback
 * ========================================================================= */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // LovyanGFX pushImage: ส่งข้อมูลภาพ RGB565 ไปยังหน้าจอ
    lcd.startWrite();
    lcd.pushImage(area->x1, area->y1, w, h, (lgfx::rgb565_t *)px_map);
    lcd.endWrite();

    // แจ้ง LVGL ว่าวาดเฟรมนี้เสร็จแล้ว
    lv_display_flush_ready(disp);
}

/* =========================================================================
 * 2. LVGL Touchpad Read Callback
 * ========================================================================= */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    bool touched = lcd.getTouch(&touchX, &touchY);

    if (!touched) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        // มีการสัมผัส -> ปลุกหน้าจอ Auto Dim ทันที
        g_backlight_mgr.on_activity();

        // ป้องกันพิกัดหลุดออกนอกขอบเขตหน้าจอ
        if (touchX >= screenWidth)  touchX = screenWidth - 1;
        if (touchY >= screenHeight) touchY = screenHeight - 1;

        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touchX;
        data->point.y = touchY;

        // อัปเดตพิกัดแบบสดบนหน้าจอ Demo
        update_touch_debug(touchX, touchY);
    }
}

/* =========================================================================
 * 3. LVGL Tick Callback
 * ========================================================================= */
static uint32_t my_tick_get_cb(void) {
    return millis();
}

/* =========================================================================
 * 4. Touch Calibration Function
 * ========================================================================= */
void run_touch_calibration(bool force) {
    prefs.begin("cyd35_touch", false);
    bool has_cal = prefs.isKey("cal_data");

    if (!has_cal || force) {
        Serial.println("\n[Touch Calibrate] Starting Touch Calibration Routine...");

        lcd.fillScreen(TFT_BLACK);
        lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        lcd.setTextDatum(lgfx::middle_center);
        lcd.setTextSize(1);

        lcd.drawString("--- TOUCH CALIBRATION ---", screenWidth / 2, 40);
        lcd.drawString("Please touch the 4 red points accurately", screenWidth / 2, 70);
        lcd.drawString("using a stylus or your finger.", screenWidth / 2, 95);

        uint16_t calData[8];
        lcd.calibrateTouch(calData, TFT_WHITE, TFT_RED, 25);

        // บันทึกพิกัดลง Flash NVS ถาวร
        prefs.putBytes("cal_data", calData, sizeof(calData));
        Serial.println("[Touch Calibrate] Calibration completed and saved to NVS!");
    } else {
        uint16_t calData[8];
        prefs.getBytes("cal_data", calData, sizeof(calData));
        lcd.setTouchCalibrate(calData);
        Serial.println("[Touch Calibrate] Loaded existing calibration from NVS.");
    }
    prefs.end();
}

// Callback เมื่อบูทระบบ PKS เสร็จสมบูรณ์ (100%)
static void on_boot_finished(void) {
    Serial.println("[Boot] Boot sequence complete. Launching OBD2 Smart Gauge Dashboard...");
    
    // สร้างหน้าจอ Screen ใหม่สำหรับ Dashboard
    lv_obj_t *dash_screen = lv_obj_create(NULL);
    obd_dashboard_init(dash_screen);
    
    // สลับหน้าจอพร้อม Fade In และ auto_del = true เพื่อคืนหน่วยความจำหน้า Boot Screen อัตโนมัติ
    lv_screen_load_anim(dash_screen, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0, true);
    
    // เริ่มต้น Mock Simulation อัปเดตข้อมูลเซนเซอร์
    obd_dashboard_start_mock_simulation(40);
    Serial.println("[UI] OBD2 Smart Gauge Dashboard is now active!\n");
}

/* =========================================================================
 * 5. Arduino Setup Function
 * ========================================================================= */
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n==============================================");
    Serial.println("  ESP32-3248S035R Base Template Initializing");
    Serial.println("==============================================");

    // ปิด RGB LED บนบอร์ด (Active LOW)
    pinMode(4, OUTPUT);
    pinMode(16, OUTPUT);
    pinMode(17, OUTPUT);
    digitalWrite(4, HIGH);
    digitalWrite(16, HIGH);
    digitalWrite(17, HIGH);

    // 1. เริ่มต้นหน้าจอ LovyanGFX
    lcd.init();
    lcd.setRotation(1);               // 1 = แนวนอน Landscape (480x320)
    lcd.setBrightness(255);           // ความสว่างเริ่มต้น 100%
    Serial.println("[LovyanGFX] Display & Touch Initialized.");

    // โหลดหรือทำการ Calibrate ทัชสกรีน
    run_touch_calibration(false);

    // เริ่มต้นระบบจัดการแสงหน้าจอ Auto Dimming (พักหน้าจออัตโนมัติ 60 วินาที)
    g_backlight_mgr.init(lcd, 60);
    Serial.println("[Backlight] Auto Dimming Backlight Manager Initialized.");

    // 2. เริ่มต้นระบบกราฟิก LVGL v9
    lv_init();
    lv_tick_set_cb(my_tick_get_cb);

    // 3. ผูก Display Driver เข้ากับ LVGL
    lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, nullptr, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    Serial.println("[LVGL v9] Display Driver Registered.");

    // 4. ผูก Touch Input Driver เข้ากับ LVGL
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);
    Serial.println("[LVGL v9] Touch Driver Registered.");

    // 5. แสดงหน้าจอ Boot / Splash Screen ของ PKS
    boot_screen_init(on_boot_finished);
    Serial.println("[UI] PKS Boot Screen started successfully!\n");
}

/* =========================================================================
 * 6. Arduino Loop Function
 * ========================================================================= */
void loop() {
    // อัปเดตสถานะ Auto Dimming & Smooth Fading
    g_backlight_mgr.update();

    // หากมีการกดปุ่ม Calibrate จากหน้าจอ UI
    if (calibrate_requested) {
        calibrate_requested = false;
        run_touch_calibration(true);
        lv_obj_invalidate(lv_screen_active());
        g_backlight_mgr.on_activity();
    }

    // ให้ LVGL ประมวลผลกราฟิกและอีเวนต์
    lv_timer_handler();
    delay(5);
}
