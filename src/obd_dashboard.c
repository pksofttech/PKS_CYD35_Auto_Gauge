/**
 * @file obd_dashboard.c
 * @brief Modern Circular Cockpit OBD2 Smart Gauge & 4x4 Inclinometer Component
 * @author Embedded Systems & Automotive GUI Engineer
 *
 * Implements the photorealistic smart gauge HUD:
 * - Center: High-Contrast Speedometer Dial (0-200 km/h) with Glowing Crimson Needle & Large Digital km/h
 * - Left: Progressive Neon RPM Arc (0-8 x1000 RPM) with Digital RPM Readout
 * - Right: 4 Clean Digital Telemetry Readouts (Coolant Temp, Turbo Boost, Battery Volt, AFR)
 * - Bottom: Fuel Level, Time/Uptime & PKS AUTO-TECH Branding
 * - Mode 2: 4x4 Off-Road Clinometer (Roll & Pitch Inclinometer Dials with Rollover Hazard Alert)
 */

#include "obd_dashboard.h"
#include <stdio.h>
#include <math.h>

/* =========================================================================
 * 1. UI Theme Color Palette (Modern Cockpit Dark High-Contrast)
 * ========================================================================= */
#define COLOR_BG_MAIN lv_color_hex(0x06080C)       /* Deep Cockpit Pitch Black */
#define COLOR_PANEL_BG lv_color_hex(0x0D1117)      /* Matte Carbon Dial Face */
#define COLOR_BORDER_SUBTLE lv_color_hex(0x1F2937) /* Sleek Dark Slate Border */
#define COLOR_ACCENT_CYAN lv_color_hex(0x00E5FF)   /* High-Vis Neon Cyan */
#define COLOR_ACCENT_BLUE lv_color_hex(0x0284C7)   /* Electric Cobalt Blue */
#define COLOR_ALERT_RED lv_color_hex(0xFF2A2A)     /* Glowing Crimson Sport Needle / Redline */
#define COLOR_WARN_AMBER lv_color_hex(0xF59E0B)    /* Warning Amber */
#define COLOR_SAFE_GREEN lv_color_hex(0x10B981)    /* Connected Emerald */
#define COLOR_TEXT_PRIMARY lv_color_hex(0xF8FAFC)  /* Crisp Pure White */
#define COLOR_TEXT_MUTED lv_color_hex(0x94A3B8)    /* Silver Slate */
#define COLOR_TEXT_DIM lv_color_hex(0x475569)      /* Dimmed Inactive */

/* =========================================================================
 * 2. Static UI Object References (Pre-allocated for Zero Dynamic Allocation)
 * ========================================================================= */
typedef struct
{
    /* Root Containers */
    lv_obj_t *root_cont;
    lv_obj_t *status_bar;
    lv_obj_t *view_cockpit;
    lv_obj_t *view_offroad;
    bool is_offroad;

    /* Status Bar Widgets */
    lv_obj_t *lbl_conn_status;
    lv_obj_t *lbl_brand_title;
    lv_obj_t *btn_mode_toggle;
    lv_obj_t *lbl_mode_toggle;
    lv_obj_t *badge_mil;
    lv_obj_t *lbl_mil;

    /* ================= PAGE 1: Modern Smart Gauge Cockpit ================= */
    /* Left Panel: Progressive Tachometer Arc (0-8 x1000 RPM) */
    lv_obj_t *panel_left_rpm;
    lv_obj_t *arc_rpm_bg;
    lv_obj_t *arc_rpm_val;
    lv_obj_t *lbl_rpm_digital;
    lv_obj_t *lbl_rpm_unit;

    /* Center Panel: Prominent Speedometer Dial (0-200 km/h) */
    lv_obj_t *panel_center_speed;
    lv_obj_t *scale_speed;
    lv_obj_t *needle_speed;
    lv_obj_t *arc_speed_val;
    lv_obj_t *hub_speed;
    lv_obj_t *lbl_speed_val;
    lv_obj_t *lbl_speed_unit;

    /* Right Panel: 4 Clean Digital Telemetry Readouts */
    lv_obj_t *panel_right_telemetry;

    lv_obj_t *lbl_ect_title;
    lv_obj_t *lbl_ect_val;
    lv_obj_t *bar_ect;

    lv_obj_t *lbl_boost_title;
    lv_obj_t *lbl_boost_val;
    lv_obj_t *bar_boost;

    lv_obj_t *lbl_batt_title;
    lv_obj_t *lbl_batt_val;
    lv_obj_t *bar_batt;

    lv_obj_t *lbl_afr_title;
    lv_obj_t *lbl_afr_val;
    lv_obj_t *bar_afr;

    /* Bottom Sub-Bar */
    lv_obj_t *cockpit_footer;
    lv_obj_t *lbl_fuel_val;
    lv_obj_t *lbl_time_val;
    lv_obj_t *lbl_bottom_brand;

#define CAR_SIDE_PTS_COUNT 22
#define CAR_FRONT_PTS_COUNT 21
#define CAR_GRILLE_PTS_COUNT 10
#define HORIZON_PTS_COUNT 2

    /* ================= PAGE 2: 4x4 Off-Road Inclinometer ================= */
    lv_obj_t *card_roll;
    lv_obj_t *scale_roll;
    lv_obj_t *needle_roll;
    lv_obj_t *arc_roll_val;
    lv_obj_t *hub_roll;
    lv_obj_t *line_car_roll;
    lv_obj_t *line_grille_roll;
    lv_obj_t *line_horizon_roll;
    lv_point_precise_t pts_car_roll[CAR_FRONT_PTS_COUNT];
    lv_point_precise_t pts_grille_roll[CAR_GRILLE_PTS_COUNT];
    lv_point_precise_t pts_horizon_roll[HORIZON_PTS_COUNT];
    lv_obj_t *lbl_roll_val;
    lv_obj_t *lbl_roll_status;

    lv_obj_t *card_pitch;
    lv_obj_t *scale_pitch;
    lv_obj_t *needle_pitch;
    lv_obj_t *arc_pitch_val;
    lv_obj_t *hub_pitch;
    lv_obj_t *line_car_pitch;
    lv_obj_t *line_horizon_pitch;
    lv_point_precise_t pts_car_pitch[CAR_SIDE_PTS_COUNT];
    lv_point_precise_t pts_horizon_pitch[HORIZON_PTS_COUNT];
    lv_obj_t *lbl_pitch_val;
    lv_obj_t *lbl_pitch_grade;

    lv_obj_t *offroad_footer;
    lv_obj_t *lbl_heading_val;
    lv_obj_t *lbl_altitude_val;
    lv_obj_t *lbl_offroad_speed;
    lv_obj_t *lbl_offroad_batt;

    /* Common Styles */
    lv_style_t style_panel_card;
    lv_style_t style_redline_major;
    lv_style_t style_redline_minor;
    bool styles_initialized;

} obd_ui_context_t;

static obd_ui_context_t g_ui;
static obd2_telemetry_t g_telemetry_cache;
static lv_timer_t *g_mock_timer = NULL;

/* Base 2D Vector Model for Suzuki Jimny JB74 Front Silhouette (Roll, 21 points) */
static const lv_point_precise_t g_base_car_front[CAR_FRONT_PTS_COUNT] = {
    {-14, -13}, {14, -13}, /* Boxy Roof Rain Gutter */
    {16, -4},              /* Right A-Pillar */
    {21, -4},
    {21, -1},
    {16, -1}, /* Right Side Mirror */
    {20, 2},
    {20, 9},
    {15, 9}, /* Right Tire & Wide Fender Flare */
    {15, 4},
    {10, 5},
    {-10, 5},
    {-15, 4}, /* Front Bumper & Skid Plate */
    {-15, 9},
    {-20, 9},
    {-20, 2}, /* Left Tire & Wide Fender Flare */
    {-16, -1},
    {-21, -1},
    {-21, -4}, /* Left Side Mirror */
    {-16, -4}, /* Left A-Pillar */
    {-14, -13} /* Loop Close */
};

/* Base 2D Vector Model for Suzuki Jimny JB74 Front Grille & Round Headlights (10 points) */
static const lv_point_precise_t g_base_car_grille[CAR_GRILLE_PTS_COUNT] = {
    {-12, 1}, {-10, -2}, {-7, -2}, {-7, 1}, /* Left Round Headlight */
    {-3, 0},
    {3, 0}, /* 5-Slot Center Grille Bar */
    {7, 1},
    {7, -2},
    {10, -2},
    {12, 1} /* Right Round Headlight */
};

/* Base 2D Vector Model for Suzuki Jimny JB74 Side Profile (Pitch, 22 points) */
static const lv_point_precise_t g_base_car_side[CAR_SIDE_PTS_COUNT] = {
    {-15, -13}, {7, -13}, /* Boxy Flat Roof */
    {13, -2},             /* Upright Windshield */
    {22, -2},             /* Flat Hood */
    {24, 3},
    {21, 6}, /* Rugged Front Bumper */
    {19, 2},
    {11, 2},
    {10, 6}, /* Front Squared Wheel Arch */
    {-5, 6}, /* Rocker Sill / Side Step */
    {-6, 2},
    {-14, 2},
    {-15, 6}, /* Rear Squared Wheel Arch */
    {-19, 6},
    {-20, 3}, /* Rear Bumper */
    {-24, 2},
    {-25, -3},
    {-25, -8},
    {-20, -8},  /* Iconic Tailgate Spare Tire */
    {-18, -3},  /* Tailgate Lower */
    {-16, -13}, /* Upright Rear Pillar */
    {-15, -13}  /* Loop Close */
};

static const lv_point_precise_t g_base_horizon[HORIZON_PTS_COUNT] = {
    {-30, 10}, {30, 10}};

static void rotate_vector_points(const lv_point_precise_t *src, lv_point_precise_t *dst, uint32_t count, float angle_deg, int32_t cx, int32_t cy)
{
    float rad = angle_deg * 3.14159265f / 180.0f;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);
    for (uint32_t i = 0; i < count; i++)
    {
        float x = (float)src[i].x;
        float y = (float)src[i].y;
        dst[i].x = (lv_value_precise_t)(cx + (x * cos_a - y * sin_a));
        dst[i].y = (lv_value_precise_t)(cy + (x * sin_a + y * cos_a));
    }
}

/* Scale Label Strings */
static const char *g_speed_labels[] = {"0", "20", "40", "60", "80", "100", "120", "140", "160", "180", "200", NULL};
static const char *g_incline_labels[] = {"-40", "-20", "0", "+20", "+40", NULL};

static void mode_toggle_event_cb(lv_event_t *e);

/* =========================================================================
 * 3. Style Initialization Helper
 * ========================================================================= */
static void obd_styles_init(void)
{
    if (g_ui.styles_initialized)
        return;

    /* Modern Panel Card Style */
    lv_style_init(&g_ui.style_panel_card);
    lv_style_set_bg_color(&g_ui.style_panel_card, COLOR_PANEL_BG);
    lv_style_set_bg_opa(&g_ui.style_panel_card, LV_OPA_COVER);
    lv_style_set_border_color(&g_ui.style_panel_card, COLOR_BORDER_SUBTLE);
    lv_style_set_border_width(&g_ui.style_panel_card, 1);
    lv_style_set_radius(&g_ui.style_panel_card, 10);
    lv_style_set_pad_all(&g_ui.style_panel_card, 4);

    /* Redline Section Major Ticks Style */
    lv_style_init(&g_ui.style_redline_major);
    lv_style_set_line_color(&g_ui.style_redline_major, COLOR_ALERT_RED);
    lv_style_set_line_width(&g_ui.style_redline_major, 3);
    lv_style_set_text_color(&g_ui.style_redline_major, COLOR_ALERT_RED);

    /* Redline Section Minor Ticks Style */
    lv_style_init(&g_ui.style_redline_minor);
    lv_style_set_line_color(&g_ui.style_redline_minor, COLOR_ALERT_RED);
    lv_style_set_line_width(&g_ui.style_redline_minor, 2);

    g_ui.styles_initialized = true;
}

/* =========================================================================
 * 4. UI Construction
 * ========================================================================= */

/**
 * @brief Construct the Status Bar
 */
static void build_status_bar(lv_obj_t *parent)
{
    g_ui.status_bar = lv_obj_create(parent);
    lv_obj_set_size(g_ui.status_bar, lv_pct(100), 26);
    lv_obj_set_style_bg_color(g_ui.status_bar, lv_color_hex(0x0A0D13), LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.status_bar, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.status_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.status_bar, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(g_ui.status_bar, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(g_ui.status_bar, 1, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.status_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(g_ui.status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_ui.status_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* 1. Connection Indicator */
    g_ui.lbl_conn_status = lv_label_create(g_ui.status_bar);
    lv_label_set_text(g_ui.lbl_conn_status, LV_SYMBOL_OK " OBD-II");
    lv_obj_set_style_text_color(g_ui.lbl_conn_status, COLOR_SAFE_GREEN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_conn_status, &lv_font_montserrat_12, LV_PART_MAIN);

    /* 2. Top Center Brand Tag */
    g_ui.lbl_brand_title = lv_label_create(g_ui.status_bar);
    lv_label_set_text(g_ui.lbl_brand_title, "PKS SMART GAUGE");
    lv_obj_set_style_text_color(g_ui.lbl_brand_title, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_brand_title, &lv_font_montserrat_12, LV_PART_MAIN);

    /* 3. Right: View Toggle Button & MIL Badge */
    lv_obj_t *r_box = lv_obj_create(g_ui.status_bar);
    lv_obj_set_size(r_box, LV_SIZE_CONTENT, 22);
    lv_obj_set_style_bg_opa(r_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(r_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(r_box, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(r_box, 6, LV_PART_MAIN);
    lv_obj_clear_flag(r_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(r_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r_box, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.btn_mode_toggle = lv_button_create(r_box);
    lv_obj_set_size(g_ui.btn_mode_toggle, 90, 20);
    lv_obj_set_style_bg_color(g_ui.btn_mode_toggle, lv_color_hex(0x1F2937), LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.btn_mode_toggle, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.btn_mode_toggle, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.btn_mode_toggle, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_ui.btn_mode_toggle, 1, LV_PART_MAIN);
    lv_obj_add_event_cb(g_ui.btn_mode_toggle, mode_toggle_event_cb, LV_EVENT_CLICKED, NULL);

    g_ui.lbl_mode_toggle = lv_label_create(g_ui.btn_mode_toggle);
    lv_label_set_text(g_ui.lbl_mode_toggle, "4x4 HUD");
    lv_obj_set_style_text_color(g_ui.lbl_mode_toggle, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_mode_toggle, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(g_ui.lbl_mode_toggle);

    g_ui.badge_mil = lv_obj_create(r_box);
    lv_obj_set_size(g_ui.badge_mil, LV_SIZE_CONTENT, 20);
    lv_obj_set_style_bg_color(g_ui.badge_mil, lv_color_hex(0x181C24), LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.badge_mil, lv_color_hex(0x283040), LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.badge_mil, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.badge_mil, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(g_ui.badge_mil, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(g_ui.badge_mil, 1, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.badge_mil, LV_OBJ_FLAG_SCROLLABLE);

    g_ui.lbl_mil = lv_label_create(g_ui.badge_mil);
    lv_label_set_text(g_ui.lbl_mil, "MIL");
    lv_obj_set_style_text_color(g_ui.lbl_mil, COLOR_TEXT_DIM, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_mil, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(g_ui.lbl_mil);
}

/**
 * @brief Construct Page 1: Modern Smart Gauge Cockpit (Exact Image Mockup Layout)
 */
static void build_cockpit_view(lv_obj_t *parent)
{
    g_ui.view_cockpit = lv_obj_create(parent);
    lv_obj_set_size(g_ui.view_cockpit, lv_pct(100), lv_pct(89));
    lv_obj_set_style_bg_opa(g_ui.view_cockpit, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(g_ui.view_cockpit, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_ui.view_cockpit, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(g_ui.view_cockpit, 2, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.view_cockpit, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(g_ui.view_cockpit, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(g_ui.view_cockpit, 1);

    /* Upper Main Tri-Section (Left RPM Arc | Center Speedometer Dial | Right Telemetry Stack) */
    lv_obj_t *tri_row = lv_obj_create(g_ui.view_cockpit);
    lv_obj_set_size(tri_row, lv_pct(100), lv_pct(89));
    lv_obj_set_style_bg_opa(tri_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(tri_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tri_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(tri_row, 4, LV_PART_MAIN);
    lv_obj_clear_flag(tri_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(tri_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tri_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* =========================================================================
     * SECTION 1: Left Progressive Tachometer Arc Panel (23% Width)
     * ========================================================================= */
    g_ui.panel_left_rpm = lv_obj_create(tri_row);
    lv_obj_set_size(g_ui.panel_left_rpm, lv_pct(23), lv_pct(100));
    lv_obj_add_style(g_ui.panel_left_rpm, &g_ui.style_panel_card, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.panel_left_rpm, LV_OBJ_FLAG_SCROLLABLE);

    /* Left Header Tag */
    lv_obj_t *lbl_tacho_tag = lv_label_create(g_ui.panel_left_rpm);
    lv_label_set_text(lbl_tacho_tag, "TACHOMETER");
    lv_obj_set_style_text_color(lbl_tacho_tag, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_tacho_tag, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_tacho_tag, LV_ALIGN_TOP_MID, 0, 2);

    /* Concentric RPM Arc - Sized properly (96x96) to fit cleanly inside panel */
    g_ui.arc_rpm_val = lv_arc_create(g_ui.panel_left_rpm);
    lv_obj_set_size(g_ui.arc_rpm_val, 96, 96);
    lv_obj_align(g_ui.arc_rpm_val, LV_ALIGN_CENTER, 0, -4);
    lv_arc_set_range(g_ui.arc_rpm_val, 0, OBD_RPM_MAX);
    lv_arc_set_value(g_ui.arc_rpm_val, 3200);
    lv_arc_set_bg_angles(g_ui.arc_rpm_val, 135, 45); /* 270 degree tachometer sweep */
    lv_arc_set_rotation(g_ui.arc_rpm_val, 0);

    lv_obj_set_style_arc_color(g_ui.arc_rpm_val, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_arc_width(g_ui.arc_rpm_val, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(g_ui.arc_rpm_val, true, LV_PART_MAIN);

    lv_obj_set_style_arc_color(g_ui.arc_rpm_val, COLOR_ACCENT_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(g_ui.arc_rpm_val, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(g_ui.arc_rpm_val, true, LV_PART_INDICATOR);

    lv_obj_remove_style(g_ui.arc_rpm_val, NULL, LV_PART_KNOB);
    lv_obj_set_style_opa(g_ui.arc_rpm_val, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(g_ui.arc_rpm_val, LV_OBJ_FLAG_CLICKABLE);

    /* Center Digital RPM readout inside Arc */
    lv_obj_t *rpm_box = lv_obj_create(g_ui.panel_left_rpm);
    lv_obj_set_size(rpm_box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(rpm_box, LV_ALIGN_CENTER, 0, -4);
    lv_obj_set_style_bg_opa(rpm_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(rpm_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(rpm_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(rpm_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(rpm_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rpm_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_rpm_digital = lv_label_create(rpm_box);
    lv_label_set_text(g_ui.lbl_rpm_digital, "3.2");
    lv_obj_set_style_text_color(g_ui.lbl_rpm_digital, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_rpm_digital, &lv_font_montserrat_24, LV_PART_MAIN);

    g_ui.lbl_rpm_unit = lv_label_create(rpm_box);
    lv_label_set_text(g_ui.lbl_rpm_unit, "x1000\nRPM");
    lv_obj_set_style_text_color(g_ui.lbl_rpm_unit, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_rpm_unit, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(g_ui.lbl_rpm_unit, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    /* Bottom Sub-indicator */
    lv_obj_t *lbl_rpm_scale = lv_label_create(g_ui.panel_left_rpm);
    lv_label_set_text(lbl_rpm_scale, "0 - 8k RPM");
    lv_obj_set_style_text_color(lbl_rpm_scale, COLOR_TEXT_DIM, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_rpm_scale, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_rpm_scale, LV_ALIGN_BOTTOM_MID, 0, -2);

    /* =========================================================================
     * SECTION 2: Center Prominent Analog Speedometer Dial Panel (47% Width)
     * ========================================================================= */
    g_ui.panel_center_speed = lv_obj_create(tri_row);
    lv_obj_set_size(g_ui.panel_center_speed, lv_pct(47), lv_pct(100));
    lv_obj_add_style(g_ui.panel_center_speed, &g_ui.style_panel_card, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.panel_center_speed, LV_OBJ_FLAG_SCROLLABLE);

    /* Speedometer Scale (0 - 200 km/h) */
    g_ui.scale_speed = lv_scale_create(g_ui.panel_center_speed);
    lv_obj_set_size(g_ui.scale_speed, lv_pct(96), lv_pct(96));
    lv_obj_center(g_ui.scale_speed);
    lv_scale_set_mode(g_ui.scale_speed, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(g_ui.scale_speed, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.scale_speed, LV_RADIUS_CIRCLE, LV_PART_MAIN);

    lv_scale_set_range(g_ui.scale_speed, 0, 200);
    lv_scale_set_angle_range(g_ui.scale_speed, 270);
    lv_scale_set_rotation(g_ui.scale_speed, 135);
    lv_scale_set_total_tick_count(g_ui.scale_speed, 21); /* 0, 10, 20... 200 */
    lv_scale_set_major_tick_every(g_ui.scale_speed, 2);
    lv_scale_set_label_show(g_ui.scale_speed, true);
    lv_scale_set_text_src(g_ui.scale_speed, g_speed_labels);

    /* Scale Ticks & Numbers Style */
    lv_obj_set_style_length(g_ui.scale_speed, 6, LV_PART_ITEMS);
    lv_obj_set_style_line_color(g_ui.scale_speed, COLOR_TEXT_DIM, LV_PART_ITEMS);
    lv_obj_set_style_line_width(g_ui.scale_speed, 2, LV_PART_ITEMS);

    lv_obj_set_style_length(g_ui.scale_speed, 12, LV_PART_INDICATOR);
    lv_obj_set_style_line_color(g_ui.scale_speed, COLOR_TEXT_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(g_ui.scale_speed, 3, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(g_ui.scale_speed, COLOR_TEXT_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_text_font(g_ui.scale_speed, &lv_font_montserrat_12, LV_PART_INDICATOR);

    /* Glowing Red Sport Needle */
    g_ui.needle_speed = lv_line_create(g_ui.scale_speed);
    lv_obj_set_style_line_color(g_ui.needle_speed, COLOR_ALERT_RED, LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.needle_speed, 4, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(g_ui.needle_speed, true, LV_PART_MAIN);
    lv_scale_set_line_needle_value(g_ui.scale_speed, g_ui.needle_speed, 72, 73);

    /* Center Progressive Speedometer Arc (0 - 200 km/h) */
    g_ui.arc_speed_val = lv_arc_create(g_ui.panel_center_speed);
    lv_obj_set_size(g_ui.arc_speed_val, 106, 106);
    lv_obj_align(g_ui.arc_speed_val, LV_ALIGN_CENTER, 0, -15);
    lv_arc_set_range(g_ui.arc_speed_val, 0, 200);
    lv_arc_set_value(g_ui.arc_speed_val, 73);
    lv_arc_set_bg_angles(g_ui.arc_speed_val, 135, 45); /* 270 degree sweep matching outer dial */
    lv_arc_set_rotation(g_ui.arc_speed_val, 0);

    lv_obj_set_style_arc_color(g_ui.arc_speed_val, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_arc_width(g_ui.arc_speed_val, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(g_ui.arc_speed_val, true, LV_PART_MAIN);

    lv_obj_set_style_arc_color(g_ui.arc_speed_val, COLOR_ALERT_RED, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(g_ui.arc_speed_val, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(g_ui.arc_speed_val, true, LV_PART_INDICATOR);

    lv_obj_remove_style(g_ui.arc_speed_val, NULL, LV_PART_KNOB);
    lv_obj_set_style_opa(g_ui.arc_speed_val, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(g_ui.arc_speed_val, LV_OBJ_FLAG_CLICKABLE);

    /* Center Digital Speedometer Hub inside Arc */
    g_ui.hub_speed = lv_obj_create(g_ui.panel_center_speed);
    lv_obj_set_size(g_ui.hub_speed, 90, 90);
    lv_obj_align(g_ui.hub_speed, LV_ALIGN_CENTER, 0, -15);
    lv_obj_set_style_radius(g_ui.hub_speed, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_ui.hub_speed, lv_color_hex(0x080B10), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_ui.hub_speed, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.hub_speed, lv_color_hex(0x1F2937), LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.hub_speed, 1, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.hub_speed, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(g_ui.hub_speed, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_ui.hub_speed, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_speed_val = lv_label_create(g_ui.hub_speed);
    lv_label_set_text(g_ui.lbl_speed_val, "73");
    lv_obj_set_style_text_color(g_ui.lbl_speed_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_speed_val, &lv_font_montserrat_32, LV_PART_MAIN);

    g_ui.lbl_speed_unit = lv_label_create(g_ui.hub_speed);
    lv_label_set_text(g_ui.lbl_speed_unit, "km/h");
    lv_obj_set_style_text_color(g_ui.lbl_speed_unit, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_speed_unit, &lv_font_montserrat_12, LV_PART_MAIN);

    /* =========================================================================
     * SECTION 3: Right Clean Digital Telemetry Column (28% Width)
     * ========================================================================= */
    g_ui.panel_right_telemetry = lv_obj_create(tri_row);
    lv_obj_set_size(g_ui.panel_right_telemetry, lv_pct(28), lv_pct(100));
    lv_obj_add_style(g_ui.panel_right_telemetry, &g_ui.style_panel_card, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.panel_right_telemetry, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(g_ui.panel_right_telemetry, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_ui.panel_right_telemetry, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(g_ui.panel_right_telemetry, 4, LV_PART_MAIN);

    /* 1. COOLANT TEMP CARD */
    lv_obj_t *box_ect = lv_obj_create(g_ui.panel_right_telemetry);
    lv_obj_set_size(box_ect, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(box_ect, lv_color_hex(0x0A0E17), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box_ect, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(box_ect, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(box_ect, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(box_ect, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(box_ect, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(box_ect, 3, LV_PART_MAIN);
    lv_obj_clear_flag(box_ect, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(box_ect, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(box_ect, 2, LV_PART_MAIN);

    lv_obj_t *row_ect = lv_obj_create(box_ect);
    lv_obj_set_size(row_ect, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_ect, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(row_ect, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(row_ect, 0, LV_PART_MAIN);
    lv_obj_clear_flag(row_ect, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(row_ect, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_ect, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_ect_title = lv_label_create(row_ect);
    lv_label_set_text(g_ui.lbl_ect_title, "COOLANT");
    lv_obj_set_style_text_color(g_ui.lbl_ect_title, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_ect_title, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_ect_val = lv_label_create(row_ect);
    lv_label_set_text(g_ui.lbl_ect_val, "92 °C");
    lv_obj_set_style_text_color(g_ui.lbl_ect_val, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_ect_val, &lv_font_montserrat_14, LV_PART_MAIN);

    g_ui.bar_ect = lv_bar_create(box_ect);
    lv_obj_set_size(g_ui.bar_ect, lv_pct(100), 5);
    lv_bar_set_range(g_ui.bar_ect, 40, 120);
    lv_bar_set_value(g_ui.bar_ect, 92, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_ui.bar_ect, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.bar_ect, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_ui.bar_ect, COLOR_ACCENT_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_ui.bar_ect, 2, LV_PART_INDICATOR);

    /* 2. BOOST CARD */
    lv_obj_t *box_boost = lv_obj_create(g_ui.panel_right_telemetry);
    lv_obj_set_size(box_boost, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(box_boost, lv_color_hex(0x0A0E17), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box_boost, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(box_boost, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(box_boost, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(box_boost, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(box_boost, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(box_boost, 3, LV_PART_MAIN);
    lv_obj_clear_flag(box_boost, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(box_boost, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(box_boost, 2, LV_PART_MAIN);

    lv_obj_t *row_boost = lv_obj_create(box_boost);
    lv_obj_set_size(row_boost, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_boost, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(row_boost, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(row_boost, 0, LV_PART_MAIN);
    lv_obj_clear_flag(row_boost, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(row_boost, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_boost, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_boost_title = lv_label_create(row_boost);
    lv_label_set_text(g_ui.lbl_boost_title, "BOOST");
    lv_obj_set_style_text_color(g_ui.lbl_boost_title, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_boost_title, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_boost_val = lv_label_create(row_boost);
    lv_label_set_text(g_ui.lbl_boost_val, "1.1 BAR");
    lv_obj_set_style_text_color(g_ui.lbl_boost_val, COLOR_ACCENT_BLUE, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_boost_val, &lv_font_montserrat_14, LV_PART_MAIN);

    g_ui.bar_boost = lv_bar_create(box_boost);
    lv_obj_set_size(g_ui.bar_boost, lv_pct(100), 5);
    lv_bar_set_range(g_ui.bar_boost, 0, 250);
    lv_bar_set_value(g_ui.bar_boost, 110, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_ui.bar_boost, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.bar_boost, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_ui.bar_boost, COLOR_ACCENT_BLUE, LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_ui.bar_boost, 2, LV_PART_INDICATOR);

    /* 3. BATT CARD */
    lv_obj_t *box_batt = lv_obj_create(g_ui.panel_right_telemetry);
    lv_obj_set_size(box_batt, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(box_batt, lv_color_hex(0x0A0E17), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box_batt, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(box_batt, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(box_batt, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(box_batt, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(box_batt, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(box_batt, 3, LV_PART_MAIN);
    lv_obj_clear_flag(box_batt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(box_batt, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(box_batt, 2, LV_PART_MAIN);

    lv_obj_t *row_batt = lv_obj_create(box_batt);
    lv_obj_set_size(row_batt, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_batt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(row_batt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(row_batt, 0, LV_PART_MAIN);
    lv_obj_clear_flag(row_batt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(row_batt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_batt, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_batt_title = lv_label_create(row_batt);
    lv_label_set_text(g_ui.lbl_batt_title, "BATT");
    lv_obj_set_style_text_color(g_ui.lbl_batt_title, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_batt_title, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_batt_val = lv_label_create(row_batt);
    lv_label_set_text(g_ui.lbl_batt_val, "14.1 V");
    lv_obj_set_style_text_color(g_ui.lbl_batt_val, COLOR_SAFE_GREEN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_batt_val, &lv_font_montserrat_14, LV_PART_MAIN);

    g_ui.bar_batt = lv_bar_create(box_batt);
    lv_obj_set_size(g_ui.bar_batt, lv_pct(100), 5);
    lv_bar_set_range(g_ui.bar_batt, 100, 160);
    lv_bar_set_value(g_ui.bar_batt, 141, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_ui.bar_batt, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.bar_batt, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_ui.bar_batt, COLOR_SAFE_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_ui.bar_batt, 2, LV_PART_INDICATOR);

    /* 4. AFR CARD */
    lv_obj_t *box_afr = lv_obj_create(g_ui.panel_right_telemetry);
    lv_obj_set_size(box_afr, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(box_afr, lv_color_hex(0x0A0E17), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box_afr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(box_afr, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(box_afr, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(box_afr, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(box_afr, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(box_afr, 3, LV_PART_MAIN);
    lv_obj_clear_flag(box_afr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(box_afr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(box_afr, 2, LV_PART_MAIN);

    lv_obj_t *row_afr = lv_obj_create(box_afr);
    lv_obj_set_size(row_afr, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_afr, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(row_afr, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(row_afr, 0, LV_PART_MAIN);
    lv_obj_clear_flag(row_afr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(row_afr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_afr, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_afr_title = lv_label_create(row_afr);
    lv_label_set_text(g_ui.lbl_afr_title, "AFR");
    lv_obj_set_style_text_color(g_ui.lbl_afr_title, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_afr_title, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_afr_val = lv_label_create(row_afr);
    lv_label_set_text(g_ui.lbl_afr_val, "14.7");
    lv_obj_set_style_text_color(g_ui.lbl_afr_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_afr_val, &lv_font_montserrat_14, LV_PART_MAIN);

    g_ui.bar_afr = lv_bar_create(box_afr);
    lv_obj_set_size(g_ui.bar_afr, lv_pct(100), 5);
    lv_bar_set_range(g_ui.bar_afr, 100, 200);
    lv_bar_set_value(g_ui.bar_afr, 147, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_ui.bar_afr, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.bar_afr, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_ui.bar_afr, COLOR_WARN_AMBER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_ui.bar_afr, 2, LV_PART_INDICATOR);

    /* =========================================================================
     * SECTION 4: Cockpit Bottom Status Bar
     * ========================================================================= */
    g_ui.cockpit_footer = lv_obj_create(g_ui.view_cockpit);
    lv_obj_set_size(g_ui.cockpit_footer, lv_pct(100), 24);
    lv_obj_set_style_bg_color(g_ui.cockpit_footer, lv_color_hex(0x0A0D13), LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.cockpit_footer, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.cockpit_footer, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.cockpit_footer, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(g_ui.cockpit_footer, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(g_ui.cockpit_footer, 1, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.cockpit_footer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(g_ui.cockpit_footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_ui.cockpit_footer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_fuel_val = lv_label_create(g_ui.cockpit_footer);
    lv_label_set_text(g_ui.lbl_fuel_val, LV_SYMBOL_TINT " 7/8");
    lv_obj_set_style_text_color(g_ui.lbl_fuel_val, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_fuel_val, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_bottom_brand = lv_label_create(g_ui.cockpit_footer);
    lv_label_set_text(g_ui.lbl_bottom_brand, "PKS AUTO-TECH");
    lv_obj_set_style_text_color(g_ui.lbl_bottom_brand, COLOR_TEXT_DIM, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_bottom_brand, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_time_val = lv_label_create(g_ui.cockpit_footer);
    lv_label_set_text(g_ui.lbl_time_val, "SYS 14:32");
    lv_obj_set_style_text_color(g_ui.lbl_time_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_time_val, &lv_font_montserrat_12, LV_PART_MAIN);
}

/**
 * @brief Construct Page 2: 4x4 Off-Road Inclinometer View
 */
static void build_offroad_view(lv_obj_t *parent)
{
    g_ui.view_offroad = lv_obj_create(parent);
    lv_obj_set_size(g_ui.view_offroad, lv_pct(100), lv_pct(89));
    lv_obj_set_style_bg_opa(g_ui.view_offroad, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(g_ui.view_offroad, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_ui.view_offroad, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(g_ui.view_offroad, 4, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.view_offroad, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(g_ui.view_offroad, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(g_ui.view_offroad, 1);
    lv_obj_add_flag(g_ui.view_offroad, LV_OBJ_FLAG_HIDDEN);

    /* Dual Inclinometer Dials Row */
    lv_obj_t *dials_row = lv_obj_create(g_ui.view_offroad);
    lv_obj_set_size(dials_row, lv_pct(100), lv_pct(78));
    lv_obj_set_style_bg_opa(dials_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(dials_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dials_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(dials_row, 6, LV_PART_MAIN);
    lv_obj_clear_flag(dials_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(dials_row, LV_FLEX_FLOW_ROW);

    /* 1. ROLL DIAL */
    g_ui.card_roll = lv_obj_create(dials_row);
    lv_obj_set_size(g_ui.card_roll, lv_pct(49), lv_pct(100));
    lv_obj_add_style(g_ui.card_roll, &g_ui.style_panel_card, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.card_roll, LV_OBJ_FLAG_SCROLLABLE);

    g_ui.scale_roll = lv_scale_create(g_ui.card_roll);
    lv_obj_set_size(g_ui.scale_roll, lv_pct(94), lv_pct(94));
    lv_obj_center(g_ui.scale_roll);
    lv_scale_set_mode(g_ui.scale_roll, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(g_ui.scale_roll, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_scale_set_range(g_ui.scale_roll, -45, 45);
    lv_scale_set_angle_range(g_ui.scale_roll, 180);
    lv_scale_set_rotation(g_ui.scale_roll, 180);
    lv_scale_set_total_tick_count(g_ui.scale_roll, 19);
    lv_scale_set_major_tick_every(g_ui.scale_roll, 4);
    lv_scale_set_label_show(g_ui.scale_roll, true);
    lv_scale_set_text_src(g_ui.scale_roll, g_incline_labels);

    lv_obj_set_style_length(g_ui.scale_roll, 6, LV_PART_ITEMS);
    lv_obj_set_style_line_color(g_ui.scale_roll, COLOR_TEXT_DIM, LV_PART_ITEMS);
    lv_obj_set_style_line_width(g_ui.scale_roll, 2, LV_PART_ITEMS);

    lv_obj_set_style_length(g_ui.scale_roll, 10, LV_PART_INDICATOR);
    lv_obj_set_style_line_color(g_ui.scale_roll, COLOR_ACCENT_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(g_ui.scale_roll, 3, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(g_ui.scale_roll, COLOR_TEXT_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_text_font(g_ui.scale_roll, &lv_font_montserrat_12, LV_PART_INDICATOR);

    g_ui.needle_roll = lv_line_create(g_ui.scale_roll);
    lv_obj_set_style_line_color(g_ui.needle_roll, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.needle_roll, 4, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(g_ui.needle_roll, true, LV_PART_MAIN);
    lv_scale_set_line_needle_value(g_ui.scale_roll, g_ui.needle_roll, 68, 0);

    /* Center Progressive Inclinometer Roll Arc (-45° to +45°) */
    g_ui.arc_roll_val = lv_arc_create(g_ui.card_roll);
    lv_obj_set_size(g_ui.arc_roll_val, 118, 118);
    lv_obj_center(g_ui.arc_roll_val);
    lv_arc_set_mode(g_ui.arc_roll_val, LV_ARC_MODE_SYMMETRICAL);
    lv_arc_set_range(g_ui.arc_roll_val, -45, 45);
    lv_arc_set_value(g_ui.arc_roll_val, 0);
    lv_arc_set_bg_angles(g_ui.arc_roll_val, 135, 45); /* 270 degree symmetrical sweep */
    lv_arc_set_rotation(g_ui.arc_roll_val, 0);

    lv_obj_set_style_arc_color(g_ui.arc_roll_val, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_arc_width(g_ui.arc_roll_val, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(g_ui.arc_roll_val, true, LV_PART_MAIN);

    lv_obj_set_style_arc_color(g_ui.arc_roll_val, COLOR_ACCENT_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(g_ui.arc_roll_val, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(g_ui.arc_roll_val, true, LV_PART_INDICATOR);

    lv_obj_remove_style(g_ui.arc_roll_val, NULL, LV_PART_KNOB);
    lv_obj_set_style_opa(g_ui.arc_roll_val, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(g_ui.arc_roll_val, LV_OBJ_FLAG_CLICKABLE);

    /* Center Roll Digital Hub inside Arc */
    g_ui.hub_roll = lv_obj_create(g_ui.card_roll);
    lv_obj_set_size(g_ui.hub_roll, 94, 94);
    lv_obj_center(g_ui.hub_roll);
    lv_obj_set_style_radius(g_ui.hub_roll, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_ui.hub_roll, lv_color_hex(0x080B10), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_ui.hub_roll, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.hub_roll, lv_color_hex(0x1F2937), LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.hub_roll, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_ui.hub_roll, 0, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.hub_roll, LV_OBJ_FLAG_SCROLLABLE);

    /* 4x4 Front Horizon Reference Line */
    g_ui.line_horizon_roll = lv_line_create(g_ui.hub_roll);
    lv_obj_set_style_line_color(g_ui.line_horizon_roll, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.line_horizon_roll, 1, LV_PART_MAIN);
    rotate_vector_points(g_base_horizon, g_ui.pts_horizon_roll, HORIZON_PTS_COUNT, 0, 47, 24);
    lv_line_set_points(g_ui.line_horizon_roll, g_ui.pts_horizon_roll, HORIZON_PTS_COUNT);

    /* Suzuki Jimny JB74 Front Rotating Vehicle Silhouette */
    g_ui.line_car_roll = lv_line_create(g_ui.hub_roll);
    lv_obj_set_style_line_color(g_ui.line_car_roll, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.line_car_roll, 2, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(g_ui.line_car_roll, true, LV_PART_MAIN);
    rotate_vector_points(g_base_car_front, g_ui.pts_car_roll, CAR_FRONT_PTS_COUNT, 0, 47, 24);
    lv_line_set_points(g_ui.line_car_roll, g_ui.pts_car_roll, CAR_FRONT_PTS_COUNT);

    /* Suzuki Jimny JB74 5-Slot Grille & Round Headlights */
    g_ui.line_grille_roll = lv_line_create(g_ui.hub_roll);
    lv_obj_set_style_line_color(g_ui.line_grille_roll, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.line_grille_roll, 1, LV_PART_MAIN);
    rotate_vector_points(g_base_car_grille, g_ui.pts_grille_roll, CAR_GRILLE_PTS_COUNT, 0, 47, 24);
    lv_line_set_points(g_ui.line_grille_roll, g_ui.pts_grille_roll, CAR_GRILLE_PTS_COUNT);

    g_ui.lbl_roll_val = lv_label_create(g_ui.hub_roll);
    lv_label_set_text(g_ui.lbl_roll_val, "0° LEVEL");
    lv_obj_set_style_text_color(g_ui.lbl_roll_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_roll_val, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(g_ui.lbl_roll_val, LV_ALIGN_TOP_MID, 0, 48);

    g_ui.lbl_roll_status = lv_label_create(g_ui.hub_roll);
    lv_label_set_text(g_ui.lbl_roll_status, LV_SYMBOL_OK " STABLE");
    lv_obj_set_style_text_color(g_ui.lbl_roll_status, COLOR_SAFE_GREEN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_roll_status, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(g_ui.lbl_roll_status, LV_ALIGN_TOP_MID, 0, 68);

    /* 2. PITCH DIAL */
    g_ui.card_pitch = lv_obj_create(dials_row);
    lv_obj_set_size(g_ui.card_pitch, lv_pct(49), lv_pct(100));
    lv_obj_add_style(g_ui.card_pitch, &g_ui.style_panel_card, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.card_pitch, LV_OBJ_FLAG_SCROLLABLE);

    g_ui.scale_pitch = lv_scale_create(g_ui.card_pitch);
    lv_obj_set_size(g_ui.scale_pitch, lv_pct(94), lv_pct(94));
    lv_obj_center(g_ui.scale_pitch);
    lv_scale_set_mode(g_ui.scale_pitch, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(g_ui.scale_pitch, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_scale_set_range(g_ui.scale_pitch, -45, 45);
    lv_scale_set_angle_range(g_ui.scale_pitch, 180);
    lv_scale_set_rotation(g_ui.scale_pitch, 180);
    lv_scale_set_total_tick_count(g_ui.scale_pitch, 19);
    lv_scale_set_major_tick_every(g_ui.scale_pitch, 4);
    lv_scale_set_label_show(g_ui.scale_pitch, true);
    lv_scale_set_text_src(g_ui.scale_pitch, g_incline_labels);

    lv_obj_set_style_length(g_ui.scale_pitch, 6, LV_PART_ITEMS);
    lv_obj_set_style_line_color(g_ui.scale_pitch, COLOR_TEXT_DIM, LV_PART_ITEMS);
    lv_obj_set_style_line_width(g_ui.scale_pitch, 2, LV_PART_ITEMS);

    lv_obj_set_style_length(g_ui.scale_pitch, 10, LV_PART_INDICATOR);
    lv_obj_set_style_line_color(g_ui.scale_pitch, COLOR_ALERT_RED, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(g_ui.scale_pitch, 3, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(g_ui.scale_pitch, COLOR_TEXT_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_text_font(g_ui.scale_pitch, &lv_font_montserrat_12, LV_PART_INDICATOR);

    g_ui.needle_pitch = lv_line_create(g_ui.scale_pitch);
    lv_obj_set_style_line_color(g_ui.needle_pitch, COLOR_ALERT_RED, LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.needle_pitch, 4, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(g_ui.needle_pitch, true, LV_PART_MAIN);
    lv_scale_set_line_needle_value(g_ui.scale_pitch, g_ui.needle_pitch, 68, 0);

    /* Center Progressive Inclinometer Pitch Arc (-45° to +45°) */
    g_ui.arc_pitch_val = lv_arc_create(g_ui.card_pitch);
    lv_obj_set_size(g_ui.arc_pitch_val, 118, 118);
    lv_obj_center(g_ui.arc_pitch_val);
    lv_arc_set_mode(g_ui.arc_pitch_val, LV_ARC_MODE_SYMMETRICAL);
    lv_arc_set_range(g_ui.arc_pitch_val, -45, 45);
    lv_arc_set_value(g_ui.arc_pitch_val, 0);
    lv_arc_set_bg_angles(g_ui.arc_pitch_val, 135, 45); /* 270 degree symmetrical sweep */
    lv_arc_set_rotation(g_ui.arc_pitch_val, 0);

    lv_obj_set_style_arc_color(g_ui.arc_pitch_val, lv_color_hex(0x131A26), LV_PART_MAIN);
    lv_obj_set_style_arc_width(g_ui.arc_pitch_val, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(g_ui.arc_pitch_val, true, LV_PART_MAIN);

    lv_obj_set_style_arc_color(g_ui.arc_pitch_val, COLOR_ACCENT_BLUE, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(g_ui.arc_pitch_val, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(g_ui.arc_pitch_val, true, LV_PART_INDICATOR);

    lv_obj_remove_style(g_ui.arc_pitch_val, NULL, LV_PART_KNOB);
    lv_obj_set_style_opa(g_ui.arc_pitch_val, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(g_ui.arc_pitch_val, LV_OBJ_FLAG_CLICKABLE);

    /* Center Pitch Digital Hub inside Arc */
    g_ui.hub_pitch = lv_obj_create(g_ui.card_pitch);
    lv_obj_set_size(g_ui.hub_pitch, 94, 94);
    lv_obj_center(g_ui.hub_pitch);
    lv_obj_set_style_radius(g_ui.hub_pitch, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_ui.hub_pitch, lv_color_hex(0x080B10), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_ui.hub_pitch, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.hub_pitch, lv_color_hex(0x1F2937), LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.hub_pitch, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_ui.hub_pitch, 0, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.hub_pitch, LV_OBJ_FLAG_SCROLLABLE);

    /* 4x4 Side Horizon Reference Line */
    g_ui.line_horizon_pitch = lv_line_create(g_ui.hub_pitch);
    lv_obj_set_style_line_color(g_ui.line_horizon_pitch, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.line_horizon_pitch, 1, LV_PART_MAIN);
    rotate_vector_points(g_base_horizon, g_ui.pts_horizon_pitch, HORIZON_PTS_COUNT, 0, 47, 24);
    lv_line_set_points(g_ui.line_horizon_pitch, g_ui.pts_horizon_pitch, HORIZON_PTS_COUNT);

    /* 4x4 Side Rotating Vehicle Silhouette */
    g_ui.line_car_pitch = lv_line_create(g_ui.hub_pitch);
    lv_obj_set_style_line_color(g_ui.line_car_pitch, COLOR_ACCENT_BLUE, LV_PART_MAIN);
    lv_obj_set_style_line_width(g_ui.line_car_pitch, 2, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(g_ui.line_car_pitch, true, LV_PART_MAIN);
    rotate_vector_points(g_base_car_side, g_ui.pts_car_pitch, CAR_SIDE_PTS_COUNT, 0, 47, 24);
    lv_line_set_points(g_ui.line_car_pitch, g_ui.pts_car_pitch, CAR_SIDE_PTS_COUNT);

    g_ui.lbl_pitch_val = lv_label_create(g_ui.hub_pitch);
    lv_label_set_text(g_ui.lbl_pitch_val, "0° LEVEL");
    lv_obj_set_style_text_color(g_ui.lbl_pitch_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_pitch_val, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(g_ui.lbl_pitch_val, LV_ALIGN_TOP_MID, 0, 48);

    g_ui.lbl_pitch_grade = lv_label_create(g_ui.hub_pitch);
    lv_label_set_text(g_ui.lbl_pitch_grade, "0% Grade");
    lv_obj_set_style_text_color(g_ui.lbl_pitch_grade, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_pitch_grade, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(g_ui.lbl_pitch_grade, LV_ALIGN_TOP_MID, 0, 68);

    /* 3. OFFROAD FOOTER */
    g_ui.offroad_footer = lv_obj_create(g_ui.view_offroad);
    lv_obj_set_size(g_ui.offroad_footer, lv_pct(100), 24);
    lv_obj_set_style_bg_color(g_ui.offroad_footer, lv_color_hex(0x0A0D13), LV_PART_MAIN);
    lv_obj_set_style_border_color(g_ui.offroad_footer, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ui.offroad_footer, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(g_ui.offroad_footer, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(g_ui.offroad_footer, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(g_ui.offroad_footer, 1, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.offroad_footer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(g_ui.offroad_footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_ui.offroad_footer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ui.lbl_heading_val = lv_label_create(g_ui.offroad_footer);
    lv_label_set_text(g_ui.lbl_heading_val, "HDG: 000° N");
    lv_obj_set_style_text_color(g_ui.lbl_heading_val, COLOR_ACCENT_CYAN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_heading_val, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_altitude_val = lv_label_create(g_ui.offroad_footer);
    lv_label_set_text(g_ui.lbl_altitude_val, "ALT: 0 m");
    lv_obj_set_style_text_color(g_ui.lbl_altitude_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_altitude_val, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_offroad_speed = lv_label_create(g_ui.offroad_footer);
    lv_label_set_text(g_ui.lbl_offroad_speed, "SPD: 0 KM/H");
    lv_obj_set_style_text_color(g_ui.lbl_offroad_speed, COLOR_SAFE_GREEN, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_offroad_speed, &lv_font_montserrat_12, LV_PART_MAIN);

    g_ui.lbl_offroad_batt = lv_label_create(g_ui.offroad_footer);
    lv_label_set_text(g_ui.lbl_offroad_batt, "BAT: 12.0 V");
    lv_obj_set_style_text_color(g_ui.lbl_offroad_batt, COLOR_TEXT_MUTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(g_ui.lbl_offroad_batt, &lv_font_montserrat_12, LV_PART_MAIN);
}

static void mode_toggle_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        obd_dashboard_set_view_mode(!g_ui.is_offroad);
    }
}

void obd_dashboard_set_view_mode(bool offroad_mode)
{
    g_ui.is_offroad = offroad_mode;
    if (g_ui.is_offroad)
    {
        lv_obj_add_flag(g_ui.view_cockpit, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_ui.view_offroad, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(g_ui.lbl_mode_toggle, "GAUGE");
    }
    else
    {
        lv_obj_add_flag(g_ui.view_offroad, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_ui.view_cockpit, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(g_ui.lbl_mode_toggle, "4x4 HUD");
    }
}

bool obd_dashboard_is_offroad_mode(void)
{
    return g_ui.is_offroad;
}

/**
 * @brief Initialize and build the responsive OBD-II Gauge Dashboard UI tree
 */
lv_obj_t *obd_dashboard_init(lv_obj_t *parent)
{
    if (!parent)
    {
        parent = lv_screen_active();
        if (!parent)
        {
            parent = lv_obj_create(NULL);
            lv_screen_load(parent);
        }
    }

    obd_styles_init();

    g_ui.root_cont = lv_obj_create(parent);
    lv_obj_set_size(g_ui.root_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(g_ui.root_cont, COLOR_BG_MAIN, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_ui.root_cont, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_opa(g_ui.root_cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_ui.root_cont, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(g_ui.root_cont, 3, LV_PART_MAIN);
    lv_obj_clear_flag(g_ui.root_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(g_ui.root_cont, LV_FLEX_FLOW_COLUMN);

    build_status_bar(g_ui.root_cont);
    build_cockpit_view(g_ui.root_cont);
    build_offroad_view(g_ui.root_cont);

    g_ui.is_offroad = false;

    return g_ui.root_cont;
}

static const char *get_cardinal_dir(uint16_t deg)
{
    static const char *dirs[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    uint8_t idx = (uint8_t)(((deg + 22) % 360) / 45);
    return dirs[idx];
}

/* =========================================================================
 * 5. High-Performance Zero-Allocation Telemetry Update
 * ========================================================================= */
void obd_dashboard_update(const obd2_telemetry_t *data)
{
    if (!data || !g_ui.root_cont)
        return;

    /* 1. Connection Status */
    if (data->connected != g_telemetry_cache.connected)
    {
        if (data->connected)
        {
            lv_label_set_text(g_ui.lbl_conn_status, LV_SYMBOL_OK " OBD-II");
            lv_obj_set_style_text_color(g_ui.lbl_conn_status, COLOR_SAFE_GREEN, LV_PART_MAIN);
        }
        else
        {
            lv_label_set_text(g_ui.lbl_conn_status, LV_SYMBOL_CLOSE " NO LINK");
            lv_obj_set_style_text_color(g_ui.lbl_conn_status, COLOR_ALERT_RED, LV_PART_MAIN);
        }
    }

    /* 2. MIL (Check Engine Warning) */
    if (data->mil_status != g_telemetry_cache.mil_status)
    {
        if (data->mil_status)
        {
            lv_label_set_text(g_ui.lbl_mil, "CHECK");
            lv_obj_set_style_text_color(g_ui.lbl_mil, COLOR_ALERT_RED, LV_PART_MAIN);
            lv_obj_set_style_bg_color(g_ui.badge_mil, lv_color_hex(0x3B1214), LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.badge_mil, COLOR_ALERT_RED, LV_PART_MAIN);
        }
        else
        {
            lv_label_set_text(g_ui.lbl_mil, "MIL");
            lv_obj_set_style_text_color(g_ui.lbl_mil, COLOR_TEXT_DIM, LV_PART_MAIN);
            lv_obj_set_style_bg_color(g_ui.badge_mil, lv_color_hex(0x181C24), LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.badge_mil, lv_color_hex(0x283040), LV_PART_MAIN);
        }
    }

    /* 3. Tachometer (RPM Arc & Digital Readout) */
    if (data->rpm != g_telemetry_cache.rpm)
    {
        uint16_t clamped_rpm = (data->rpm > OBD_RPM_MAX) ? OBD_RPM_MAX : data->rpm;
        lv_arc_set_value(g_ui.arc_rpm_val, clamped_rpm);

        float rpm_k = (float)clamped_rpm / 1000.0f;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", rpm_k);
        lv_label_set_text(g_ui.lbl_rpm_digital, buf);

        if (clamped_rpm >= OBD_RPM_REDLINE)
        {
            lv_obj_set_style_arc_color(g_ui.arc_rpm_val, COLOR_ALERT_RED, LV_PART_INDICATOR);
            lv_obj_set_style_text_color(g_ui.lbl_rpm_digital, COLOR_ALERT_RED, LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_arc_color(g_ui.arc_rpm_val, COLOR_ACCENT_CYAN, LV_PART_INDICATOR);
            lv_obj_set_style_text_color(g_ui.lbl_rpm_digital, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
        }
    }

    /* 4. Speedometer (Scale Needle, Progressive Arc & Digital Readout) */
    if (data->speed != g_telemetry_cache.speed)
    {
        uint16_t clamped_spd = (data->speed > 200) ? 200 : data->speed;
        if (g_ui.scale_speed && g_ui.needle_speed)
        {
            lv_scale_set_line_needle_value(g_ui.scale_speed, g_ui.needle_speed, 72, clamped_spd);
        }
        if (g_ui.arc_speed_val)
        {
            lv_arc_set_value(g_ui.arc_speed_val, clamped_spd);
        }

        char buf[16];
        snprintf(buf, sizeof(buf), "%u", clamped_spd);
        lv_label_set_text(g_ui.lbl_speed_val, buf);

        snprintf(buf, sizeof(buf), "SPD: %u KM/H", clamped_spd);
        lv_label_set_text(g_ui.lbl_offroad_speed, buf);
    }

    /* 5. Coolant Temperature (ECT) */
    if (data->coolant_temp != g_telemetry_cache.coolant_temp)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d °C", data->coolant_temp);
        lv_label_set_text(g_ui.lbl_ect_val, buf);

        int32_t ect_val = data->coolant_temp;
        if (ect_val < 40)
            ect_val = 40;
        if (ect_val > 120)
            ect_val = 120;
        if (g_ui.bar_ect)
            lv_bar_set_value(g_ui.bar_ect, ect_val, LV_ANIM_OFF);

        if (data->coolant_temp > OBD_COOLANT_OVERHEAT_THRESH)
        {
            lv_obj_set_style_text_color(g_ui.lbl_ect_val, COLOR_ALERT_RED, LV_PART_MAIN);
            if (g_ui.bar_ect)
                lv_obj_set_style_bg_color(g_ui.bar_ect, COLOR_ALERT_RED, LV_PART_INDICATOR);
        }
        else
        {
            lv_obj_set_style_text_color(g_ui.lbl_ect_val, COLOR_ACCENT_CYAN, LV_PART_MAIN);
            if (g_ui.bar_ect)
                lv_obj_set_style_bg_color(g_ui.bar_ect, COLOR_ACCENT_CYAN, LV_PART_INDICATOR);
        }
    }

    /* 6. Turbo Boost / MAP */
    if (fabsf(data->map_bar - g_telemetry_cache.map_bar) > 0.01f)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f BAR", data->map_bar);
        lv_label_set_text(g_ui.lbl_boost_val, buf);

        int32_t boost_val = (int32_t)(data->map_bar * 100.0f);
        if (boost_val < 0)
            boost_val = 0;
        if (boost_val > 250)
            boost_val = 250;
        if (g_ui.bar_boost)
            lv_bar_set_value(g_ui.bar_boost, boost_val, LV_ANIM_OFF);
    }

    /* 7. Battery Voltage */
    if (fabsf(data->voltage - g_telemetry_cache.voltage) > 0.05f)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f V", data->voltage);
        lv_label_set_text(g_ui.lbl_batt_val, buf);
        lv_label_set_text(g_ui.lbl_offroad_batt, buf);

        int32_t batt_val = (int32_t)(data->voltage * 10.0f);
        if (batt_val < 100)
            batt_val = 100;
        if (batt_val > 160)
            batt_val = 160;
        if (g_ui.bar_batt)
            lv_bar_set_value(g_ui.bar_batt, batt_val, LV_ANIM_OFF);

        if (data->voltage < OBD_VOLTAGE_LOW_THRESH || data->voltage > OBD_VOLTAGE_HIGH_THRESH)
        {
            lv_obj_set_style_text_color(g_ui.lbl_batt_val, COLOR_WARN_AMBER, LV_PART_MAIN);
            if (g_ui.bar_batt)
                lv_obj_set_style_bg_color(g_ui.bar_batt, COLOR_WARN_AMBER, LV_PART_INDICATOR);
        }
        else
        {
            lv_obj_set_style_text_color(g_ui.lbl_batt_val, COLOR_SAFE_GREEN, LV_PART_MAIN);
            if (g_ui.bar_batt)
                lv_obj_set_style_bg_color(g_ui.bar_batt, COLOR_SAFE_GREEN, LV_PART_INDICATOR);
        }
    }

    /* 8. Air-Fuel Ratio (AFR) */
    if (fabsf(data->afr - g_telemetry_cache.afr) > 0.05f)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", data->afr);
        lv_label_set_text(g_ui.lbl_afr_val, buf);

        int32_t afr_val = (int32_t)(data->afr * 10.0f);
        if (afr_val < 100)
            afr_val = 100;
        if (afr_val > 200)
            afr_val = 200;
        if (g_ui.bar_afr)
            lv_bar_set_value(g_ui.bar_afr, afr_val, LV_ANIM_OFF);
    }

    /* 9. Fuel Level */
    if (data->fuel_pct != g_telemetry_cache.fuel_pct)
    {
        char buf[16];
        if (data->fuel_pct >= 85)
            snprintf(buf, sizeof(buf), LV_SYMBOL_TINT " 7/8");
        else if (data->fuel_pct >= 60)
            snprintf(buf, sizeof(buf), LV_SYMBOL_TINT " 3/4");
        else if (data->fuel_pct >= 40)
            snprintf(buf, sizeof(buf), LV_SYMBOL_TINT " 1/2");
        else if (data->fuel_pct >= 20)
            snprintf(buf, sizeof(buf), LV_SYMBOL_TINT " 1/4");
        else
            snprintf(buf, sizeof(buf), LV_SYMBOL_WARNING " LOW");
        lv_label_set_text(g_ui.lbl_fuel_val, buf);
    }

    /* 10. 4x4 Off-Road Inclinometer: ROLL Angle */
    if (fabsf(data->roll_deg - g_telemetry_cache.roll_deg) > 0.2f)
    {
        float roll = data->roll_deg;
        if (roll < -45.0f)
            roll = -45.0f;
        if (roll > 45.0f)
            roll = 45.0f;

        if (g_ui.scale_roll && g_ui.needle_roll)
        {
            lv_scale_set_line_needle_value(g_ui.scale_roll, g_ui.needle_roll, 68, (int32_t)roll);
        }
        if (g_ui.arc_roll_val)
        {
            lv_arc_set_value(g_ui.arc_roll_val, (int32_t)roll);
        }

        /* Dynamically rotate Suzuki Jimny JB74 Front Silhouette & Grille */
        if (g_ui.line_car_roll)
        {
            rotate_vector_points(g_base_car_front, g_ui.pts_car_roll, CAR_FRONT_PTS_COUNT, roll, 47, 24);
            lv_line_set_points(g_ui.line_car_roll, g_ui.pts_car_roll, CAR_FRONT_PTS_COUNT);
        }
        if (g_ui.line_grille_roll)
        {
            rotate_vector_points(g_base_car_grille, g_ui.pts_grille_roll, CAR_GRILLE_PTS_COUNT, roll, 47, 24);
            lv_line_set_points(g_ui.line_grille_roll, g_ui.pts_grille_roll, CAR_GRILLE_PTS_COUNT);
        }

        char buf[32];
        if (fabsf(roll) < 1.0f)
        {
            snprintf(buf, sizeof(buf), "0° LEVEL");
        }
        else if (roll > 0)
        {
            snprintf(buf, sizeof(buf), "R %.0f° " LV_SYMBOL_RIGHT, roll);
        }
        else
        {
            snprintf(buf, sizeof(buf), LV_SYMBOL_LEFT " L %.0f°", fabsf(roll));
        }
        lv_label_set_text(g_ui.lbl_roll_val, buf);

        float abs_roll = fabsf(roll);
        if (abs_roll >= OFFROAD_ROLL_DANGER_THRESH)
        {
            lv_obj_set_style_text_color(g_ui.lbl_roll_val, COLOR_ALERT_RED, LV_PART_MAIN);
            lv_label_set_text(g_ui.lbl_roll_status, LV_SYMBOL_WARNING " ROLLOVER");
            lv_obj_set_style_text_color(g_ui.lbl_roll_status, COLOR_ALERT_RED, LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.card_roll, COLOR_ALERT_RED, LV_PART_MAIN);
            if (g_ui.arc_roll_val)
                lv_obj_set_style_arc_color(g_ui.arc_roll_val, COLOR_ALERT_RED, LV_PART_INDICATOR);
            if (g_ui.line_car_roll)
                lv_obj_set_style_line_color(g_ui.line_car_roll, COLOR_ALERT_RED, LV_PART_MAIN);
            if (g_ui.line_grille_roll)
                lv_obj_set_style_line_color(g_ui.line_grille_roll, COLOR_ALERT_RED, LV_PART_MAIN);
        }
        else if (abs_roll >= OFFROAD_ROLL_WARN_THRESH)
        {
            lv_obj_set_style_text_color(g_ui.lbl_roll_val, COLOR_WARN_AMBER, LV_PART_MAIN);
            lv_label_set_text(g_ui.lbl_roll_status, LV_SYMBOL_WARNING " CAUTION");
            lv_obj_set_style_text_color(g_ui.lbl_roll_status, COLOR_WARN_AMBER, LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.card_roll, COLOR_WARN_AMBER, LV_PART_MAIN);
            if (g_ui.arc_roll_val)
                lv_obj_set_style_arc_color(g_ui.arc_roll_val, COLOR_WARN_AMBER, LV_PART_INDICATOR);
            if (g_ui.line_car_roll)
                lv_obj_set_style_line_color(g_ui.line_car_roll, COLOR_WARN_AMBER, LV_PART_MAIN);
            if (g_ui.line_grille_roll)
                lv_obj_set_style_line_color(g_ui.line_grille_roll, COLOR_WARN_AMBER, LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_text_color(g_ui.lbl_roll_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
            lv_label_set_text(g_ui.lbl_roll_status, LV_SYMBOL_OK " STABLE");
            lv_obj_set_style_text_color(g_ui.lbl_roll_status, COLOR_SAFE_GREEN, LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.card_roll, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
            if (g_ui.arc_roll_val)
                lv_obj_set_style_arc_color(g_ui.arc_roll_val, COLOR_ACCENT_CYAN, LV_PART_INDICATOR);
            if (g_ui.line_car_roll)
                lv_obj_set_style_line_color(g_ui.line_car_roll, COLOR_ACCENT_CYAN, LV_PART_MAIN);
            if (g_ui.line_grille_roll)
                lv_obj_set_style_line_color(g_ui.line_grille_roll, COLOR_ACCENT_CYAN, LV_PART_MAIN);
        }
    }

    /* 11. 4x4 Off-Road Inclinometer: PITCH Angle */
    if (fabsf(data->pitch_deg - g_telemetry_cache.pitch_deg) > 0.2f)
    {
        float pitch = data->pitch_deg;
        if (pitch < -45.0f)
            pitch = -45.0f;
        if (pitch > 45.0f)
            pitch = 45.0f;

        if (g_ui.scale_pitch && g_ui.needle_pitch)
        {
            lv_scale_set_line_needle_value(g_ui.scale_pitch, g_ui.needle_pitch, 68, (int32_t)pitch);
        }
        if (g_ui.arc_pitch_val)
        {
            lv_arc_set_value(g_ui.arc_pitch_val, (int32_t)pitch);
        }

        /* Dynamically rotate 4x4 Side Profile Silhouette */
        if (g_ui.line_car_pitch)
        {
            rotate_vector_points(g_base_car_side, g_ui.pts_car_pitch, CAR_SIDE_PTS_COUNT, pitch, 47, 24);
            lv_line_set_points(g_ui.line_car_pitch, g_ui.pts_car_pitch, CAR_SIDE_PTS_COUNT);
        }

        char buf[32];
        if (fabsf(pitch) < 1.0f)
        {
            snprintf(buf, sizeof(buf), "0° LEVEL");
        }
        else if (pitch > 0)
        {
            snprintf(buf, sizeof(buf), LV_SYMBOL_UP " UP +%.0f°", pitch);
        }
        else
        {
            snprintf(buf, sizeof(buf), LV_SYMBOL_DOWN " DN %.0f°", pitch);
        }
        lv_label_set_text(g_ui.lbl_pitch_val, buf);

        float grade_pct = tanf(pitch * 3.14159265f / 180.0f) * 100.0f;
        snprintf(buf, sizeof(buf), "%.1f%% Grade", fabsf(grade_pct));
        lv_label_set_text(g_ui.lbl_pitch_grade, buf);

        float abs_pitch = fabsf(pitch);
        if (abs_pitch >= OFFROAD_PITCH_DANGER_THRESH)
        {
            lv_obj_set_style_text_color(g_ui.lbl_pitch_val, COLOR_ALERT_RED, LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.card_pitch, COLOR_ALERT_RED, LV_PART_MAIN);
            if (g_ui.arc_pitch_val)
                lv_obj_set_style_arc_color(g_ui.arc_pitch_val, COLOR_ALERT_RED, LV_PART_INDICATOR);
            if (g_ui.line_car_pitch)
                lv_obj_set_style_line_color(g_ui.line_car_pitch, COLOR_ALERT_RED, LV_PART_MAIN);
        }
        else if (abs_pitch >= OFFROAD_PITCH_WARN_THRESH)
        {
            lv_obj_set_style_text_color(g_ui.lbl_pitch_val, COLOR_WARN_AMBER, LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.card_pitch, COLOR_WARN_AMBER, LV_PART_MAIN);
            if (g_ui.arc_pitch_val)
                lv_obj_set_style_arc_color(g_ui.arc_pitch_val, COLOR_WARN_AMBER, LV_PART_INDICATOR);
            if (g_ui.line_car_pitch)
                lv_obj_set_style_line_color(g_ui.line_car_pitch, COLOR_WARN_AMBER, LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_text_color(g_ui.lbl_pitch_val, COLOR_TEXT_PRIMARY, LV_PART_MAIN);
            lv_obj_set_style_border_color(g_ui.card_pitch, COLOR_BORDER_SUBTLE, LV_PART_MAIN);
            if (g_ui.arc_pitch_val)
                lv_obj_set_style_arc_color(g_ui.arc_pitch_val, COLOR_ACCENT_BLUE, LV_PART_INDICATOR);
            if (g_ui.line_car_pitch)
                lv_obj_set_style_line_color(g_ui.line_car_pitch, COLOR_ACCENT_BLUE, LV_PART_MAIN);
        }
    }

    /* 12. Compass Heading & Altitude */
    if (data->heading_deg != g_telemetry_cache.heading_deg)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "HDG: %u° %s",
                 data->heading_deg, get_cardinal_dir(data->heading_deg));
        lv_label_set_text(g_ui.lbl_heading_val, buf);
    }

    if (fabsf(data->altitude_m - g_telemetry_cache.altitude_m) > 1.0f)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "ALT: %.0f m", data->altitude_m);
        lv_label_set_text(g_ui.lbl_altitude_val, buf);
    }

    g_telemetry_cache = *data;
}

const obd2_telemetry_t *obd_dashboard_get_current_data(void)
{
    return &g_telemetry_cache;
}

/* =========================================================================
 * 6. Dynamic Mock Telemetry Simulator Task
 * ========================================================================= */
static void mock_sim_timer_cb(lv_timer_t *timer)
{
    static uint32_t sim_tick = 0;
    static uint8_t gear = 1;
    static float sim_rpm = 1000.0f;
    static float sim_speed = 0.0f;
    static float sim_coolant = 88.0f;
    static bool accelerating = true;

    sim_tick++;

    if (accelerating)
    {
        sim_rpm += 100.0f;
        sim_speed += (0.55f * (float)gear);

        if (sim_rpm >= 6200.0f)
        {
            if (gear < 5)
            {
                gear++;
                sim_rpm = 3200.0f;
            }
            else
            {
                accelerating = false;
            }
        }
    }
    else
    {
        sim_rpm -= 130.0f;
        sim_speed -= 0.75f;
        if (sim_rpm <= 1800.0f)
        {
            if (gear > 1)
            {
                gear--;
                sim_rpm = 3600.0f;
            }
            else
            {
                sim_rpm = 1000.0f;
                sim_speed = 0.0f;
                accelerating = true;
            }
        }
    }

    uint8_t tps = accelerating ? (uint8_t)(25 + (sim_rpm / 130.0f)) : 5;
    if (tps > 100)
        tps = 100;

    /* Turbo Boost (0.2 to 1.4 BAR under load) */
    float boost = 0.2f + ((float)tps / 100.0f) * 1.2f;

    /* AFR (14.7 cruise, 12.5 WOT acceleration, 16.0 overrun) */
    float afr = accelerating ? (14.7f - ((float)tps / 100.0f) * 2.2f) : 15.5f;

    static bool cooling_fan_on = false;
    if (!cooling_fan_on)
    {
        sim_coolant += 0.03f;
        if (sim_coolant >= 101.5f)
            cooling_fan_on = true;
    }
    else
    {
        sim_coolant -= 0.05f;
        if (sim_coolant <= 89.0f)
            cooling_fan_on = false;
    }

    float volt = 14.15f + 0.12f * sinf((float)sim_tick * 0.1f);
    bool mil = ((sim_tick / 200) % 2 == 1);

    /* 4x4 Offroad Simulation */
    float sim_roll = 22.0f * sinf((float)sim_tick * 0.035f) + 10.0f * sinf((float)sim_tick * 0.08f);
    float sim_pitch = 26.0f * cosf((float)sim_tick * 0.025f) + 6.0f * sinf((float)sim_tick * 0.06f);
    float sim_alt = 850.0f + 350.0f * sinf((float)sim_tick * 0.01f);
    uint16_t sim_hdg = (uint16_t)((sim_tick * 2) % 360);

    obd2_telemetry_t packet = {
        .rpm = (uint16_t)sim_rpm,
        .speed = (uint8_t)sim_speed,
        .coolant_temp = (int16_t)sim_coolant,
        .map_bar = boost,
        .tps_pct = tps,
        .voltage = volt,
        .afr = afr,
        .fuel_pct = 88,
        .mil_status = mil,
        .connected = true,
        .pitch_deg = sim_pitch,
        .roll_deg = sim_roll,
        .altitude_m = sim_alt,
        .heading_deg = sim_hdg};

    obd_dashboard_update(&packet);
}

void obd_dashboard_start_mock_simulation(uint32_t interval_ms)
{
    if (!g_mock_timer)
    {
        g_mock_timer = lv_timer_create(mock_sim_timer_cb, interval_ms, NULL);
    }
}

void obd_dashboard_stop_mock_simulation(void)
{
    if (g_mock_timer)
    {
        lv_timer_delete(g_mock_timer);
        g_mock_timer = NULL;
    }
}
