// lang.c
#include "lang.h"
#include "panel/rt_led.h"
#include "treadmill.h"

const char *strings[RT_LED_CONFIG_LNG_TOTALS][STR_TOTALS] = {
    [RT_LED_CONFIG_LNG_PT_BR] = {
        "APERTE START OU ESCOLHA UM PROGRAMA",
        "50%% DA META",
        "EMERGENCIA",
        "PAUSA * APERTE STOP PARA ENCERRAR",
        "P1 * TEMPO",
        "P2 * DIST",
        "P3 * CAL",
        "P4 * INT",
        "T1",
        "T2",
        "V1",
        "V2",
        "SERIES",
        "P5 * 30 MIN",
        "P6 * MOD",
        "IDADE",
        "TEMPO",
        "P7 * VIG",
        "P8 * 8 MIN",
        "P9 * 200 CAL",
        "PROG",
        "MAX",
        "MED",
        "META CONCLUIDA"
    },
    [RT_LED_CONFIG_LNG_EN_US] = {
        "PRESS START OR CHOOSE A PROGRAM",
        "50%% OF GOAL",
        "EMERGENCY",
        "PAUSE * PRESS STOP TO FINISH",
        "P1 * TIME",
        "P2 * DIST",
        "P3 * CAL",
        "P4 * INT",
        "T1",
        "T2",
        "S1",
        "S2",
        "SETS",
        "P5 * 30 MIN",
        "P6 * MOD",
        "AGE",
        "TIME",
        "P7 * VIG",
        "P8 * 8 MIN",
        "P9 * 200 CAL",
        "PROG",
        "MAX",
        "AVG",
        "GOAL COMPLETED"
    },
    [RT_LED_CONFIG_LNG_ES_ES] = {
        "PRESIONE START O ELIJA UN PROGRAMA",
        "50%% DE LA META",
        "EMERGENCIA",
        "PAUSA * PRESIONE STOP PARA TERMINAR",
        "P1 * TIEMPO",
        "P2 * DIST",
        "P3 * CAL",
        "P4 * INT",
        "T1",
        "T2",
        "V1",
        "V2",
        "SERIES",
        "P5 * 30 MIN",
        "P6 * MOD",
        "EDAD",
        "TIEMPO",
        "P7 * VIG",
        "P8 * 8 MIN",
        "P9 * 200 CAL",
        "PROG",
        "MAX",
        "MED",
        "META COMPLETADA"
    }
};

// void set_language(rt_led_config_language_params_t lang) {
//     if (lang >= 0 && lang < RT_LED_CONFIG_LNG_TOTALS) {
//         treadmill->config.language = lang;
//     }
// }

// set_language(LANG_EN);

// printf("%s\n", get_string(STR_PRESS_START));       // EN
// printf("%s\n", get_string(STR_META_CONCLUIDA));    // EN

// set_language(LANG_ES);
// printf("%s\n", get_string(STR_50_PERCENT));        // ES

// set_language(LANG_PT);
// printf("%s\n", get_string(STR_PROG));              // PT
