/**
 * @file obd_dashboard.h
 * @brief OBD2 Multi-Function Smart Gauge Dashboard for LVGL (v9.x)
 * @author Embedded Systems & Automotive GUI Engineer
 * 
 * Standard Compliance:
 * - SAE J1979 / ISO 15765-4 (OBD-II over CAN)
 * - Resolution-Agnostic Responsive Layout via LVGL Flexbox & Dynamic Percentages (lv_pct)
 */

#ifndef OBD_DASHBOARD_H
#define OBD_DASHBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <lvgl.h>

/* =========================================================================
 * 1. OBD-II SAE J1979 Standard PID Definitions & Thresholds
 * ========================================================================= */
#define OBD_PID_ENGINE_COOLANT_TEMP  0x05  /**< PID 0105: Coolant Temp (A - 40) °C */
#define OBD_PID_INTAKE_MAP           0x0B  /**< PID 010B: Intake Manifold Abs Pressure (A) kPa */
#define OBD_PID_ENGINE_RPM           0x0C  /**< PID 010C: Engine RPM ((A*256)+B)/4 */
#define OBD_PID_VEHICLE_SPEED        0x0D  /**< PID 010D: Speed (A) km/h */
#define OBD_PID_THROTTLE_POS         0x11  /**< PID 0111: Throttle % (A*100)/255 */

/* Automotive Thresholds & Safety Limits */
#define OBD_RPM_MAX                  8000  /**< Gauge Tachometer Maximum (RPM) */
#define OBD_RPM_REDLINE              5500  /**< Tachometer Redline Threshold (RPM) */
#define OBD_SPEED_MAX                255   /**< Maximum Speed (km/h) */
#define OBD_COOLANT_OVERHEAT_THRESH  98    /**< Overheat Alert Warning Threshold (°C) */
#define OBD_VOLTAGE_LOW_THRESH       11.8f /**< Low Battery Alert (V) */
#define OBD_VOLTAGE_HIGH_THRESH      15.0f /**< Alternator Overvoltage Alert (V) */

/* 4x4 Off-Road Inclinometer Thresholds */
#define OFFROAD_ROLL_WARN_THRESH     25.0f /**< Caution Amber Alert Roll Angle (°) */
#define OFFROAD_ROLL_DANGER_THRESH   35.0f /**< Critical Red Alert Rollover Angle (°) */
#define OFFROAD_PITCH_WARN_THRESH    28.0f /**< Steep Incline Caution Pitch Angle (°) */
#define OFFROAD_PITCH_DANGER_THRESH  38.0f /**< Extreme Grade Hazard Pitch Angle (°) */

/* =========================================================================
 * 2. OBD-II & Off-Road Telemetry Data Model
 * ========================================================================= */
typedef struct {
    uint16_t rpm;          /**< Engine Speed (RPM, 0 - 8000) [PID 010C] */
    uint8_t  speed;        /**< Vehicle Speed (km/h, 0 - 255) [PID 010D] */
    int16_t  coolant_temp; /**< Engine Coolant Temp (°C, -40 to +215) [PID 0105] */
    float    map_bar;      /**< Turbo Boost / MAP Pressure (bar) [PID 010B] */
    uint8_t  tps_pct;      /**< Throttle Position (0 - 100 %) [PID 0111] */
    float    voltage;      /**< Electrical Battery Voltage (V) */
    float    afr;          /**< Air-Fuel Ratio (AFR: e.g. 14.7 Stoichiometric) [PID 0124/0134] */
    uint8_t  fuel_pct;     /**< Fuel Tank Level % (0 - 100 %) [PID 012F] */
    bool     mil_status;   /**< Malfunction Indicator Lamp (MIL / Check Engine) */
    bool     connected;    /**< ECU Connection State (true = Connected, false = No Data) */

    /* 4x4 Off-Road Inclinometer & Compass Telemetry (IMU 6-DOF / Gyro / Accel) */
    float    pitch_deg;    /**< Pitch Angle (°: +Uphill Climb, -Downhill Descent, -45° to +45°) */
    float    roll_deg;     /**< Roll Angle (°: +Right Bank, -Left Bank, -45° to +45°) */
    float    altitude_m;   /**< Altitude (m above sea level) */
    uint16_t heading_deg;  /**< Compass Heading (0 - 359°) */
} obd2_telemetry_t;

/* =========================================================================
 * 3. Public GUI API Functions
 * ========================================================================= */

/**
 * @brief Initialize and build the responsive OBD-II Gauge Dashboard UI tree
 * @param parent Parent LVGL object (e.g., lv_screen_active()), or NULL to create on active screen
 * @return Pointer to the root container object
 */
lv_obj_t *obd_dashboard_init(lv_obj_t *parent);

/**
 * @brief Switch dashboard display between Sports Cockpit Gauge and 4x4 Offroad Inclinometer
 * @param offroad_mode true for Off-Road Inclinometer HUD, false for Sports Cockpit Gauge
 */
void obd_dashboard_set_view_mode(bool offroad_mode);

/**
 * @brief Get current dashboard view mode
 * @return true if currently showing 4x4 Offroad Inclinometer, false if Cockpit Gauge
 */
bool obd_dashboard_is_offroad_mode(void);

/**
 * @brief Thread-safe update of all Dashboard widgets with new OBD-II telemetry
 * @note Reuses existing static LVGL objects; performs zero dynamic memory allocations
 * @param data Pointer to latest telemetry dataset
 */
void obd_dashboard_update(const obd2_telemetry_t *data);

/**
 * @brief Start the built-in mock telemetry simulator (sweeping RPM, speed, ECT, TPS, etc.)
 * @param interval_ms Update period in milliseconds (recommended: 30 - 50ms for smooth 30-60 FPS)
 */
void obd_dashboard_start_mock_simulation(uint32_t interval_ms);

/**
 * @brief Stop the mock telemetry simulator
 */
void obd_dashboard_stop_mock_simulation(void);

/**
 * @brief Get the current simulated or live telemetry buffer
 * @return const pointer to current telemetry data
 */
const obd2_telemetry_t *obd_dashboard_get_current_data(void);

#ifdef __cplusplus
}
#endif

#endif /* OBD_DASHBOARD_H */
