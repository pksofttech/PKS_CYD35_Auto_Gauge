/**
 * @file lv_conf.h
 * Configuration file for LVGL v9.x
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_CONF_MINIMAL 0

#if 1 /* Set to "1" to enable content */

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 16

/*=========================
   MEMORY SETTINGS
 *=========================*/
#define LV_USE_STDLIB_MALLOC LV_STDLIB_BUILTIN
#define LV_MEM_SIZE (64 * 1024U)          /* 64 KB for LVGL internal heap */
#define LV_MEM_POOL_EXPAND_SIZE 0
#define LV_MEM_ADR 0

/*=========================
   HAL SETTINGS
 *=========================*/
#define LV_DEF_REFR_PERIOD 16             /* ~60 FPS update rate (ms) */
#define LV_DPI_DEF 130                    /* Approximate DPI for CYD 3.5" */

/*=========================
   OPERATING SYSTEM
 *=========================*/
#define LV_USE_OS LV_OS_NONE

/*=========================
   LOG SETTINGS
 *=========================*/
#define LV_USE_LOG 1
#if LV_USE_LOG
  #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
  #define LV_LOG_PRINTF 1
#endif

/*=========================
   WIDGETS & COMPONENTS
 *=========================*/
#define LV_USE_BUTTON 1
#define LV_USE_LABEL 1
#define LV_USE_IMAGE 1
#define LV_USE_LINE 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_BAR 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_ROLLER 1
#define LV_USE_TEXTAREA 1
#define LV_USE_BUTTONMATRIX 1
#define LV_USE_KEYBOARD 1
#define LV_USE_CANVAS 1
#define LV_USE_QRCODE 1

#define LV_USE_ARC 1
#define LV_USE_SCALE 1

/*=========================
   FONTS
 *=========================*/
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#endif /* End of configuration */

#endif /* LV_CONF_H */
