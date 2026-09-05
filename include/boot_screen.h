/**
 * @file boot_screen.h
 * @brief Automotive High-Tech Boot / Splash Screen for PKS Auto Gauge
 * @author Embedded Systems & Automotive GUI Engineer
 * 
 * Features:
 * - High-tech PKS Automotive Branding with Glowing Badges
 * - Smooth Diagnostic Boot Progress (Hardware Init, CAN Bus, ECU Link)
 * - Automatic Transition / Gauge Startup Sequence
 */

#ifndef BOOT_SCREEN_H
#define BOOT_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <lvgl.h>

/**
 * @brief Callback function type invoked when the boot sequence is finished
 */
typedef void (*boot_complete_cb_t)(void);

/**
 * @brief Initialize and display the PKS Boot Splash Screen
 * @param on_complete Callback to trigger when boot progress reaches 100%
 * @return Pointer to the boot screen object
 */
lv_obj_t *boot_screen_init(boot_complete_cb_t on_complete);

/**
 * @brief Manually stop/dismiss the boot screen immediately
 */
void boot_screen_dismiss(void);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_SCREEN_H */
