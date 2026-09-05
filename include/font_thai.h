/**
 * @file font_thai.h
 * @brief Multilingual Font Declarations for LVGL (Noto Sans Thai + Roboto)
 * 
 * Supports:
 * - English & ASCII: 0x20 - 0x7E (A-Z, a-z, 0-9, Symbols & Punctuation)
 * - Latin-1 Supplement: 0x00A0 - 0x00FF
 * - Thai: 0x0E01 - 0x0E5B (ก-ฮ, สระบน/ล่าง, วรรณยุกต์, เลขไทย ๐-๙, ฿)
 */

#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

LV_FONT_DECLARE(font_thai_14);
LV_FONT_DECLARE(font_thai_18);
LV_FONT_DECLARE(font_thai_24);

#ifdef __cplusplus
}
#endif
