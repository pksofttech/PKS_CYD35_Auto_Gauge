/**
 * @file backlight_manager.hpp
 * @brief ระบบจัดการไฟส่องสว่างหน้าจอ (Backlight Manager) & Auto Dimming สำหรับ ESP32 CYD 3.5"
 * 
 * คุณสมบัติ:
 * - ปรับลดแสงสว่างอัตโนมัติ (Auto Dim) เมื่อไม่มีการสัมผัสตามเวลาที่กำหนด เพื่อประหยัดพลังงานและยืดอายุจอ LCD
 * - Smooth Fading: ค่อยๆ หรี่ไฟลงและค่อยๆ สว่างขึ้นอย่างนุ่มนวล
 * - Instant Wake-up: เมื่อสัมผัสหน้าจอ จะตื่นกลับมาสว่างเต็มที่ 100% ทันที
 * - รองรับการตั้งเวลา: 30 วินาที, 1 นาที, 2 นาที, 5 นาที, หรือ ปิด (Never Dim)
 */

#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "LGFX_ESP32_3248S035.hpp"

class BacklightManager {
public:
    enum DimState {
        STATE_ACTIVE,       // จอสว่างเต็มที่ (100% = 255)
        STATE_FADING_DOWN,  // กำลังค่อยๆ หรี่ไฟลง
        STATE_DIMMED,       // หรี่ไฟลงโหมดประหยัดพลังงาน (~10% = 25)
        STATE_FADING_UP     // กำลังค่อยๆ สว่างขึ้น
    };

private:
    LGFX *_lcd = nullptr;
    uint32_t _timeout_ms = 60000;       // เวลา Idle ก่อนเริ่มหรี่ (Default 60 วินาที)
    uint32_t _last_activity = 0;        // Timestamp ล่าสุดที่มีการสัมผัสหรือทำรายการ
    uint8_t _max_brightness = 255;      // ความสว่างสูงสุด (100%)
    uint8_t _dim_brightness = 25;       // ความสว่างตอนหรี่ (~10% ยังมองเห็น แต่ลดความร้อน/กินไฟลด 85%)
    uint8_t _current_brightness = 255;  // ความสว่างปัจจุบัน
    uint8_t _target_brightness = 255;   // ความสว่างเป้าหมาย
    uint32_t _last_fade_step_time = 0;  // เวลาของสเต็ป Fade ล่าสุด
    const uint32_t FADE_STEP_INTERVAL = 15; // อัปเดตทุก 15ms
    const uint8_t FADE_STEP_AMOUNT = 6;     // เพิ่ม/ลด ทีละ 6 หน่วย (Smooth Fade ~600ms)
    DimState _state = STATE_ACTIVE;
    bool _enabled = true;

public:
    BacklightManager() {}

    /**
     * @brief เริ่มต้นระบบ Backlight Manager
     */
    void init(LGFX &lcd_instance, uint32_t timeout_sec = 60) {
        _lcd = &lcd_instance;
        set_timeout_sec(timeout_sec);
        _last_activity = millis();
        _current_brightness = _max_brightness;
        _target_brightness = _max_brightness;
        _state = STATE_ACTIVE;
        if (_lcd) {
            _lcd->setBrightness(_current_brightness);
        }
    }

    /**
     * @brief เรียกเมื่อมีการสัมผัสหน้าจอ หรือมีกิจกรรมในระบบ เพื่อรีเซ็ตเวลา Idle และปลุกหน้าจอ
     */
    void on_activity() {
        _last_activity = millis();

        // หากจอกำลังหรี่อยู่หรือกำลัง Fade ลง ให้ปลุกกลับมาสว่างเต็มที่ทันที
        if (_state != STATE_ACTIVE) {
            _target_brightness = _max_brightness;
            _current_brightness = _max_brightness;
            _state = STATE_ACTIVE;
            if (_lcd) {
                _lcd->setBrightness(_current_brightness);
            }
        }
    }

    /**
     * @brief อัปเดตสถานะ Auto Dimming และการ Fade แสง (เรียกวนลูปใน loop())
     */
    void update() {
        if (!_enabled || !_lcd || _timeout_ms == 0) return;

        uint32_t now = millis();

        // 1. ตรวจสอบว่าไม่มีการใช้งานเกินเวลา Timeout หรือยัง
        if (_state == STATE_ACTIVE) {
            if (now - _last_activity >= _timeout_ms) {
                _state = STATE_FADING_DOWN;
                _target_brightness = _dim_brightness;
            }
        }

        // 2. ปรับระดับความสว่างแบบ Smooth Fading
        if (now - _last_fade_step_time >= FADE_STEP_INTERVAL) {
            _last_fade_step_time = now;

            if (_current_brightness != _target_brightness) {
                if (_current_brightness > _target_brightness) {
                    if (_current_brightness - _target_brightness <= FADE_STEP_AMOUNT) {
                        _current_brightness = _target_brightness;
                    } else {
                        _current_brightness -= FADE_STEP_AMOUNT;
                    }
                } else {
                    if (_target_brightness - _current_brightness <= FADE_STEP_AMOUNT) {
                        _current_brightness = _target_brightness;
                    } else {
                        _current_brightness += FADE_STEP_AMOUNT;
                    }
                }

                _lcd->setBrightness(_current_brightness);

                if (_current_brightness == _target_brightness) {
                    if (_target_brightness == _dim_brightness) {
                        _state = STATE_DIMMED;
                    } else if (_target_brightness == _max_brightness) {
                        _state = STATE_ACTIVE;
                    }
                }
            }
        }
    }

    /**
     * @brief ตั้งเวลา Timeout (วินาที), ถ้าใส่ 0 จะถือว่าปิด Auto Dim
     */
    void set_timeout_sec(uint32_t sec) {
        _timeout_ms = sec * 1000;
        _enabled = (sec > 0);
        on_activity();
    }

    uint32_t get_timeout_sec() const {
        return _timeout_ms / 1000;
    }

    /**
     * @brief วนสลับค่า Timeout สำหรับปุ่มลัดใน Settings (30s -> 60s -> 120s -> 300s -> 0/ปิด)
     */
    uint32_t cycle_timeout() {
        uint32_t cur = get_timeout_sec();
        uint32_t next = 60;

        if (cur == 30)       next = 60;   // 1 นาที
        else if (cur == 60)  next = 120;  // 2 นาที
        else if (cur == 120) next = 300;  // 5 นาที
        else if (cur == 300) next = 0;    // ปิด (เปิดตลอด)
        else                 next = 30;   // 30 วินาที

        set_timeout_sec(next);
        return next;
    }

    /**
     * @brief ข้อความแสดงผลสำหรับค่า Timeout ปัจจุบัน
     */
    String get_timeout_label() const {
        uint32_t s = get_timeout_sec();
        if (s == 0) return "เปิดตลอด (Never)";
        if (s < 60) return String(s) + " วินาที";
        return String(s / 60) + " นาที";
    }

    bool is_dimmed() const {
        return (_state == STATE_DIMMED || _state == STATE_FADING_DOWN);
    }

    uint8_t get_brightness() const {
        return _current_brightness;
    }
};

// Global Instance ของ BacklightManager
inline BacklightManager g_backlight_mgr;
