/**
 * @file demo_ui.hpp
 * @brief Demo UI Template สำหรับ ESP32 CYD 3.5" (480x320 ST7796 + LVGL v9)
 * 
 * เหมาะสำหรับใช้เป็นต้นแบบเริ่มต้นสร้าง UI:
 * - ตัวอย่างปุ่มกดพร้อมนับจำนวน (Counter Button)
 * - สวิตช์เปิด/ปิด (Toggle Switch)
 * - สไลเดอร์ปรับระดับ (Slider) พร้อมแสดงค่า % แบบ Real-time
 * - ตัวอย่างฟอนต์ภาษาไทย 3 ขนาด (font_thai_14, font_thai_18, font_thai_24)
 * - แสดงสถานะระบบ: Free RAM, Uptime, พิกัด Touch X/Y ล่าสุด
 * - ปุ่ม Calibrate ทัชสกรีนในตัว
 */

#pragma once

#include <Arduino.h>
#include <lvgl.h>
#include "font_thai.h"

// ตัวแปร UI Widgets
static lv_obj_t *lbl_btn_counter = nullptr;
static lv_obj_t *lbl_slider_val = nullptr;
static lv_obj_t *lbl_switch_state = nullptr;
static lv_obj_t *lbl_touch_pos = nullptr;
static lv_obj_t *lbl_sys_info = nullptr;
static uint32_t g_click_count = 0;

// Flag สำหรับขอทำ Touch Calibration
extern bool calibrate_requested;

/* =========================================================================
 * Callbacks
 * ========================================================================= */

// Event: กดปุ่มนับเลข
static void btn_counter_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        g_click_count++;
        char buf[32];
        snprintf(buf, sizeof(buf), "กดแล้ว: %u ครั้ง", g_click_count);
        lv_label_set_text(lbl_btn_counter, buf);
    }
}

// Event: เลื่อน Slider
static void slider_event_cb(lv_event_t *e) {
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    char buf[32];
    snprintf(buf, sizeof(buf), "ระดับ: %ld %%", val);
    lv_label_set_text(lbl_slider_val, buf);
}

// Event: สลับ Switch ON/OFF
static void switch_event_cb(lv_event_t *e) {
    lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
    bool is_on = lv_obj_has_state(sw, LV_STATE_CHECKED);
    lv_label_set_text(lbl_switch_state, is_on ? "สถานะ: เปิด (ON)" : "สถานะ: ปิด (OFF)");
    lv_obj_set_style_text_color(lbl_switch_state, is_on ? lv_color_hex(0x22C55E) : lv_color_hex(0xEF4444), LV_PART_MAIN);
}

// Event: กดปุ่ม Calibrate
static void btn_cal_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        calibrate_requested = true;
    }
}

// Timer Callback: อัปเดต Uptime และ Free RAM ทุกๆ 1 วินาที
static void sys_timer_cb(lv_timer_t *timer) {
    if (!lbl_sys_info) return;

    uint32_t sec = millis() / 1000;
    uint32_t m = (sec / 60) % 60;
    uint32_t h = (sec / 3600);
    uint32_t s = sec % 60;

    char buf[64];
    snprintf(buf, sizeof(buf), "RAM ว่าง: %u KB | เวลา: %02u:%02u:%02u", 
             ESP.getFreeHeap() / 1024, h, m, s);
    lv_label_set_text(lbl_sys_info, buf);
}

/**
 * @brief ฟังก์ชันอัปเดตพิกัด Touch บนหน้าจอ (เรียกจาก touchpad read callback)
 */
inline void update_touch_debug(int16_t x, int16_t y) {
    if (lbl_touch_pos) {
        char buf[32];
        snprintf(buf, sizeof(buf), "พิกัดทัช: X=%d, Y=%d", x, y);
        lv_label_set_text(lbl_touch_pos, buf);
    }
}

/**
 * @brief ฟังก์ชันสร้างหน้าจอ Demo UI
 */
inline void create_demo_ui(void) {
    // 1. พื้นหลังหลัก (Theme Dark Slate)
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x0F172A), LV_PART_MAIN);

    // =========================================================================
    // 2. HEADER BAR (464 x 42 px)
    // =========================================================================
    lv_obj_t *header = lv_obj_create(lv_screen_active());
    lv_obj_set_size(header, 464, 42);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_border_color(header, lv_color_hex(0x334155), LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 6, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Title
    lv_obj_t *lbl_title = lv_label_create(header);
    lv_label_set_text(lbl_title, LV_SYMBOL_HOME " ESP32 CYD 3.5\" Base Template");
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x38BDF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_title, &font_thai_18, LV_PART_MAIN);
    lv_obj_align(lbl_title, LV_ALIGN_LEFT_MID, 6, 0);

    // ปุ่ม Calibrate
    lv_obj_t *btn_cal = lv_button_create(header);
    lv_obj_set_size(btn_cal, 110, 30);
    lv_obj_align(btn_cal, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_cal, lv_color_hex(0x475569), LV_PART_MAIN);
    lv_obj_set_style_radius(btn_cal, 6, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_cal, btn_cal_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_cal = lv_label_create(btn_cal);
    lv_label_set_text(lbl_cal, LV_SYMBOL_REFRESH " Calibrate");
    lv_obj_set_style_text_font(lbl_cal, &font_thai_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_cal, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(lbl_cal);

    // =========================================================================
    // 3. LEFT PANEL: Interactive Controls (228 x 256 px)
    // =========================================================================
    lv_obj_t *card_left = lv_obj_create(lv_screen_active());
    lv_obj_set_size(card_left, 228, 256);
    lv_obj_align(card_left, LV_ALIGN_TOP_LEFT, 8, 54);
    lv_obj_set_style_bg_color(card_left, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_left, lv_color_hex(0x334155), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_left, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_left, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card_left, 8, LV_PART_MAIN);
    lv_obj_clear_flag(card_left, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_ctrl_title = lv_label_create(card_left);
    lv_label_set_text(lbl_ctrl_title, "ปุ่มและการควบคุม");
    lv_obj_set_style_text_color(lbl_ctrl_title, lv_color_hex(0xF8FAFC), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_ctrl_title, &font_thai_18, LV_PART_MAIN);
    lv_obj_align(lbl_ctrl_title, LV_ALIGN_TOP_LEFT, 4, 2);

    // 1. Counter Button
    lv_obj_t *btn_count = lv_button_create(card_left);
    lv_obj_set_size(btn_count, 206, 44);
    lv_obj_align(btn_count, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_style_bg_color(btn_count, lv_color_hex(0x0284C7), LV_PART_MAIN);
    lv_obj_set_style_radius(btn_count, 8, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_count, btn_counter_cb, LV_EVENT_CLICKED, NULL);

    lbl_btn_counter = lv_label_create(btn_count);
    lv_label_set_text(lbl_btn_counter, "กดนับเลข: 0");
    lv_obj_set_style_text_font(lbl_btn_counter, &font_thai_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_btn_counter, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(lbl_btn_counter);

    // 2. Switch
    lv_obj_t *sw = lv_switch_create(card_left);
    lv_obj_set_size(sw, 56, 28);
    lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 4, 88);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lbl_switch_state = lv_label_create(card_left);
    lv_label_set_text(lbl_switch_state, "สถานะ: เปิด (ON)");
    lv_obj_set_style_text_font(lbl_switch_state, &font_thai_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_switch_state, lv_color_hex(0x22C55E), LV_PART_MAIN);
    lv_obj_align(lbl_switch_state, LV_ALIGN_TOP_LEFT, 68, 92);

    // 3. Slider
    lbl_slider_val = lv_label_create(card_left);
    lv_label_set_text(lbl_slider_val, "ระดับ: 50 %");
    lv_obj_set_style_text_font(lbl_slider_val, &font_thai_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_slider_val, lv_color_hex(0x94A3B8), LV_PART_MAIN);
    lv_obj_align(lbl_slider_val, LV_ALIGN_TOP_LEFT, 4, 130);

    lv_obj_t *slider = lv_slider_create(card_left);
    lv_obj_set_size(slider, 206, 16);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 154);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // =========================================================================
    // 4. RIGHT PANEL: System Info & Typography (228 x 256 px)
    // =========================================================================
    lv_obj_t *card_right = lv_obj_create(lv_screen_active());
    lv_obj_set_size(card_right, 228, 256);
    lv_obj_align(card_right, LV_ALIGN_TOP_RIGHT, -8, 54);
    lv_obj_set_style_bg_color(card_right, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_right, lv_color_hex(0x334155), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_right, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_right, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card_right, 8, LV_PART_MAIN);
    lv_obj_clear_flag(card_right, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_info_title = lv_label_create(card_right);
    lv_label_set_text(lbl_info_title, "ข้อมูลและฟอนต์ไทย");
    lv_obj_set_style_text_color(lbl_info_title, lv_color_hex(0xF8FAFC), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_info_title, &font_thai_18, LV_PART_MAIN);
    lv_obj_align(lbl_info_title, LV_ALIGN_TOP_LEFT, 4, 2);

    // ตัวอย่างฟอนต์ขนาดใหญ่ 24pt
    lv_obj_t *lbl_f24 = lv_label_create(card_right);
    lv_label_set_text(lbl_f24, "฿ 9,999.00");
    lv_obj_set_style_text_font(lbl_f24, &font_thai_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_f24, lv_color_hex(0x10B981), LV_PART_MAIN);
    lv_obj_align(lbl_f24, LV_ALIGN_TOP_LEFT, 4, 30);

    // ตัวอย่างฟอนต์ปกติ 14pt
    lv_obj_t *lbl_f14 = lv_label_create(card_right);
    lv_label_set_text(lbl_f14, "รองรับภาษาไทย + สัญลักษณ์\n" LV_SYMBOL_WIFI " WiFi  " LV_SYMBOL_BLUETOOTH " BLE  " LV_SYMBOL_BATTERY_FULL " Battery");
    lv_obj_set_style_text_font(lbl_f14, &font_thai_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_f14, lv_color_hex(0xCBD5E1), LV_PART_MAIN);
    lv_obj_align(lbl_f14, LV_ALIGN_TOP_LEFT, 4, 66);

    // พิกัดทัชสกรีน Live
    lbl_touch_pos = lv_label_create(card_right);
    lv_label_set_text(lbl_touch_pos, "พิกัดทัช: แตะหน้าจอเพื่อดูค่า");
    lv_obj_set_style_text_font(lbl_touch_pos, &font_thai_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_touch_pos, lv_color_hex(0x38BDF8), LV_PART_MAIN);
    lv_obj_align(lbl_touch_pos, LV_ALIGN_TOP_LEFT, 4, 116);

    // ข้อมูล RAM และเวลา Uptime
    lbl_sys_info = lv_label_create(card_right);
    lv_label_set_text(lbl_sys_info, "กำลังโหลดสถานะระบบ...");
    lv_obj_set_style_text_font(lbl_sys_info, &font_thai_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_sys_info, lv_color_hex(0x94A3B8), LV_PART_MAIN);
    lv_obj_align(lbl_sys_info, LV_ALIGN_TOP_LEFT, 4, 142);

    // คำแนะนำ Auto Dim
    lv_obj_t *lbl_dim_hint = lv_label_create(card_right);
    lv_label_set_text(lbl_dim_hint, "Auto Dim: พักหน้าจออัตโนมัติ 60s\n(แตะหน้าจอเพื่อปลุกทันที)");
    lv_obj_set_style_text_font(lbl_dim_hint, &font_thai_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl_dim_hint, lv_color_hex(0xFBBF24), LV_PART_MAIN);
    lv_obj_align(lbl_dim_hint, LV_ALIGN_BOTTOM_LEFT, 4, -4);

    // เริ่ม LVGL Timer อัปเดตทุก 1 วินาที
    lv_timer_create(sys_timer_cb, 1000, NULL);
}
