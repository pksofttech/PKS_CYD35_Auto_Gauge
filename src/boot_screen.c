/**
 * @file boot_screen.c
 * @brief PKS Automotive System Boot & Splash Screen Implementation
 * @author Embedded Systems & Automotive GUI Engineer
 */

#include "boot_screen.h"
#include <stdio.h>

/* UI Theme Colors */
#define COLOR_BOOT_BG       lv_color_hex(0x0A0C10)
#define COLOR_PKS_CYAN      lv_color_hex(0x00E5FF)
#define COLOR_PKS_BLUE      lv_color_hex(0x0284C7)
#define COLOR_CARD_BORDER   lv_color_hex(0x1E293B)
#define COLOR_TEXT_MUTED    lv_color_hex(0x64748B)
#define COLOR_TEXT_DIM      lv_color_hex(0x475569)

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *center_card;
    lv_obj_t *lbl_brand_tag;
    lv_obj_t *lbl_brand_pks;
    lv_obj_t *lbl_brand_sub;
    lv_obj_t *bar_progress;
    lv_obj_t *lbl_status;
    lv_obj_t *lbl_pct;
    lv_obj_t *footer_cont;

    lv_timer_t *timer_boot;
    boot_complete_cb_t on_complete;
    int32_t progress;
} boot_context_t;

static boot_context_t g_boot;

/* Diagnostic boot log messages corresponding to percentage steps */
static const struct {
    int32_t threshold;
    const char *msg;
} g_boot_steps[] = {
    { 0,  "Hardware Self-Check: ESP32 240MHz Dual-Core OK" },
    { 15, "Display Controller: ST7796 480x320 HSPI Ready" },
    { 35, "CAN Interface: ISO 15765-4 (500 kbps) Init..." },
    { 55, "OBD-II Protocol: Querying Active ECU PIDs..." },
    { 75, "Sensor Calibration: Tachometer & MAP Boost OK" },
    { 90, "System Ready: Loading Cockpit Telemetry HUD..." },
    { 100,"PKS Engine Intelligence Initialized." }
};

#define BOOT_STEPS_COUNT (sizeof(g_boot_steps) / sizeof(g_boot_steps[0]))

static void update_status_text(int32_t val) {
    const char *msg = g_boot_steps[0].msg;
    for (size_t i = 0; i < BOOT_STEPS_COUNT; i++) {
        if (val >= g_boot_steps[i].threshold) {
            msg = g_boot_steps[i].msg;
        }
    }
    if (g_boot.lbl_status) {
        lv_label_set_text(g_boot.lbl_status, msg);
    }
}

static void boot_timer_cb(lv_timer_t *timer) {
    (void)timer;
    g_boot.progress += 2; /* Progress step */

    if (g_boot.progress > 100) {
        g_boot.progress = 100;
    }

    /* Update Progress Bar & Labels */
    if (g_boot.bar_progress) {
        lv_bar_set_value(g_boot.bar_progress, g_boot.progress, LV_ANIM_OFF);
    }

    if (g_boot.lbl_pct) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%ld %%", (long)g_boot.progress);
        lv_label_set_text(g_boot.lbl_pct, buf);
    }

    update_status_text(g_boot.progress);

    /* Boot Sequence Finished */
    if (g_boot.progress >= 100) {
        if (g_boot.timer_boot) {
            lv_timer_delete(g_boot.timer_boot);
            g_boot.timer_boot = NULL;
        }

        boot_complete_cb_t cb = g_boot.on_complete;
        if (cb) {
            cb();
        }
    }
}

void boot_screen_dismiss(void) {
    if (g_boot.timer_boot) {
        lv_timer_delete(g_boot.timer_boot);
        g_boot.timer_boot = NULL;
    }
    if (g_boot.screen) {
        lv_obj_delete_async(g_boot.screen);
        g_boot.screen = NULL;
    }
}

lv_obj_t *boot_screen_init(boot_complete_cb_t on_complete) {
    g_boot.on_complete = on_complete;
    g_boot.progress = 0;

    /* 1. Create Boot Screen Object */
    g_boot.screen = lv_obj_create(NULL);
    lv_obj_set_size(g_boot.screen, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(g_boot.screen, COLOR_BOOT_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_boot.screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(g_boot.screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Flex Column Root */
    lv_obj_set_flex_flow(g_boot.screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_boot.screen, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(g_boot.screen, 16, LV_PART_MAIN);

    /* 2. Top Header Spacer / Version Badge */
    lv_obj_t *top_bar = lv_obj_create(g_boot.screen);
    lv_obj_set_size(top_bar, lv_pct(100), 24);
    lv_obj_set_style_bg_opa(top_bar, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(top_bar, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(top_bar, 0, LV_PART_MAIN);
    lv_obj_clear_flag(top_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *lbl_sys = lv_label_create(top_bar);
    lv_label_set_text(lbl_sys, LV_SYMBOL_SETTINGS " PKS AUTO GAUGE SYSTEM");
    lv_obj_set_style_text_color(lbl_sys, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_sys, &lv_font_montserrat_12, LV_PART_MAIN);

    lv_obj_t *lbl_ver = lv_label_create(top_bar);
    lv_label_set_text(lbl_ver, "FIRMWARE v1.0.0");
    lv_obj_set_style_text_color(lbl_ver, COLOR_TEXT_DIM, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_ver, &lv_font_montserrat_12, LV_PART_MAIN);

    /* 3. Center Branding Card (Responsive Flex) */
    g_boot.center_card = lv_obj_create(g_boot.screen);
    lv_obj_set_size(g_boot.center_card, lv_pct(88), lv_pct(60));
    lv_obj_set_style_bg_color(g_boot.center_card, lv_color_hex(0x12151D), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_boot.center_card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(g_boot.center_card, COLOR_PKS_BLUE, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_boot.center_card, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(g_boot.center_card, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_boot.center_card, 12, LV_PART_MAIN);
    lv_obj_clear_flag(g_boot.center_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(g_boot.center_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_boot.center_card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(g_boot.center_card, 4, LV_PART_MAIN);

    /* Sub-tag Badge */
    g_boot.lbl_brand_tag = lv_label_create(g_boot.center_card);
    lv_label_set_text(g_boot.lbl_brand_tag, "- PERFORMANCE TELEMETRY -");
    lv_obj_set_style_text_color(g_boot.lbl_brand_tag, COLOR_PKS_BLUE, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_boot.lbl_brand_tag, &lv_font_montserrat_12, LV_PART_MAIN);

    /* Big "PKS" Brand Logo Label */
    g_boot.lbl_brand_pks = lv_label_create(g_boot.center_card);
    lv_label_set_text(g_boot.lbl_brand_pks, "P K S");
    lv_obj_set_style_text_color(g_boot.lbl_brand_pks, COLOR_PKS_CYAN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_boot.lbl_brand_pks, &lv_font_montserrat_40, LV_PART_MAIN);

    /* Subtitle */
    g_boot.lbl_brand_sub = lv_label_create(g_boot.center_card);
    lv_label_set_text(g_boot.lbl_brand_sub, "SMART AUTOMOTIVE COCKPIT GAUGE");
    lv_obj_set_style_text_color(g_boot.lbl_brand_sub, lv_color_hex(0xCBD5E1), LV_PART_MAIN);
    lv_obj_set_style_text_font(g_boot.lbl_brand_sub, &lv_font_montserrat_14, LV_PART_MAIN);

    /* 4. Boot Progress & Diagnostic Logs Area */
    lv_obj_t *diag_box = lv_obj_create(g_boot.screen);
    lv_obj_set_size(diag_box, lv_pct(88), 54);
    lv_obj_set_style_bg_opa(diag_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(diag_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(diag_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(diag_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(diag_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(diag_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(diag_box, 4, LV_PART_MAIN);

    /* Row for Status Text and % */
    lv_obj_t *stat_row = lv_obj_create(diag_box);
    lv_obj_set_size(stat_row, lv_pct(100), 20);
    lv_obj_set_style_bg_opa(stat_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(stat_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(stat_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(stat_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(stat_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stat_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_boot.lbl_status = lv_label_create(stat_row);
    lv_label_set_text(g_boot.lbl_status, g_boot_steps[0].msg);
    lv_obj_set_style_text_color(g_boot.lbl_status, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_boot.lbl_status, &lv_font_montserrat_12, LV_PART_MAIN);

    g_boot.lbl_pct = lv_label_create(stat_row);
    lv_label_set_text(g_boot.lbl_pct, "0 %");
    lv_obj_set_style_text_color(g_boot.lbl_pct, COLOR_PKS_CYAN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_boot.lbl_pct, &lv_font_montserrat_12, LV_PART_MAIN);

    /* Progress Bar */
    g_boot.bar_progress = lv_bar_create(diag_box);
    lv_obj_set_size(g_boot.bar_progress, lv_pct(100), 6);
    lv_bar_set_range(g_boot.bar_progress, 0, 100);
    lv_bar_set_value(g_boot.bar_progress, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_boot.bar_progress, lv_color_hex(0x1B212D), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_boot.bar_progress, COLOR_PKS_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_boot.bar_progress, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(g_boot.bar_progress, 3, LV_PART_INDICATOR);

    /* Load screen into active display */
    lv_screen_load(g_boot.screen);

    /* Create animation timer (running every 40ms -> ~2.2s total boot time) */
    g_boot.timer_boot = lv_timer_create(boot_timer_cb, 40, NULL);

    return g_boot.screen;
}
