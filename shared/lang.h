#ifndef LANG_H
#define LANG_H

// Language options for Aria treadmill

#include "panel/rt_led.h"

enum {
    STR_PRESS_START,
    STR_50_PERCENT,
    STR_EMERGENCY,
    STR_PAUSE_STOP,
    STR_P1_TEMPO,
    STR_P2_DIST,
    STR_P3_CAL,
    STR_P4_INT,
    STR_T1,
    STR_T2,
    STR_V1,
    STR_V2,
    STR_SERIES,
    STR_P5_30MIN,
    STR_P6_MOD,
    STR_IDADE,
    STR_TEMPO,
    STR_P7_VIG,
    STR_P8_8MIN,
    STR_P9_200CAL,
    STR_PROG,
    STR_MAX,
    STR_MED,
    STR_META_CONCLUIDA,
    STR_HRM,
    STR_ERR,
    STR_WHEEL,
    STR_TOTALS
};

extern const char *strings[RT_LED_CONFIG_LNG_TOTALS][STR_TOTALS];
extern rt_led_config_language_params_t language;

#endif // LANG_H
