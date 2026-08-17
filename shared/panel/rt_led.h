#ifndef RT_LED_H
#define RT_LED_H

#include "../scene/scene.h"

// todo: scope?
#include "../dm/dm_render.h"
#include "../sd/sd_render.h"

#define MAJOR 1
#define MINOR 2
#define REVISION 8 ////

extern uint16_t fw_version;

typedef enum scene_config_option {
  CONFIG_OPTION_MODEL = 0,       // MDL
  CONFIG_OPTION_LANGUAGE,        // LNG
  CONFIG_OPTION_UNIT,            // UNT
  CONFIG_OPTION_WEIGHT,          // WGT
  CONFIG_OPTION_MA,              // MA
  // CONFIG_OPTION_LUB,             // LUB
  CONFIG_OPTION_NAME,           // NAM
  CONFIG_OPTION_VER,             // VER
  CONFIG_OPTION_TOTAL_DISTANCE,  // TD
  CONFIG_OPTION_TOTAL_HOURS,     // THR
  CONFIG_OPTION_BLE_STATE,       // S/BL
  CONFIG_OPTION_MODBUS,          // MBUS
  CONFIG_OPTION_TEST,            // TEST
  CONFIG_OPTION_C2V,             // C2V
  CONFIG_OPTION_ERRORS,          // ERR
  CONFIG_OPTION_MAX_TIME,        // TEMP
  CONFIG_OPTION_TOTAL
} scene_config_option_t;

typedef enum rt_led_model {
  RT_LED_MODEL_150_FREQ = 0,
  RT_LED_MODEL_150_USART,
  RT_LED_MODEL_250_USART,
  RT_LED_MODEL_TOTALS,
} rt_led_model_t;

typedef enum rt_led_weight_range {
  RT_LED_WEIGHT_MIN = 40,
  RT_LED_WEIGHT_MAX = 200,
} rt_led_weight_range_t;

typedef enum rt_led_lub_params {
  RT_LED_LUB_MIN_DISTANCE = 100,
  RT_LED_LUB_MAX_DISTANCE = 9900,
  RT_LED_LUB_DEFAULT_DISTANCE = 1000,
  RT_LED_LUB_STEP_DISTANCE = 100,
} rt_led_lub_params_t;

typedef enum rt_led_config_modbus_params {
  RT_LED_CONFIG_MODBUS_HIDE_PARAMS = 0,
  RT_LED_CONFIG_MODBUS_SHOW_PARAMS = 1,
  RT_LED_CONFIG_MODBUS_TOTALS,
} rt_led_config_modbus_params_t;

typedef enum rt_led_config_test_params {
  RT_LED_CONFIG_TEST_DISABLE = 0,
  RT_LED_CONFIG_TEST_ENABLE = 1,
  RT_LED_CONFIG_TEST_TOTALS,
} rt_led_config_test_params;

typedef enum rt_led_config_c2v_params {
  RT_LED_CONFIG_C2V_DISABLE = 0,
  RT_LED_CONFIG_C2V_ENABLE = 1,
  RT_LED_CONFIG_C2V_TOTALS,
} rt_led_config_c2v_params_t;

// Language added to Aria treadmill
typedef enum rt_led_config_language_params {
  RT_LED_CONFIG_LNG_PT_BR = 0,
  RT_LED_CONFIG_LNG_EN_US = 1,
  RT_LED_CONFIG_LNG_ES_ES = 2,
  RT_LED_CONFIG_LNG_TOTALS = 3,
} rt_led_config_language_params_t;

typedef enum rt_led_config_unit_params {
  RT_LED_CONFIG_UNIT_KM = 0,    // km
  RT_LED_CONFIG_UNIT_MILES = 1, // milhas
  RT_LED_CONFIG_UNIT_TOTALS
} rt_led_config_unit_params_t;

void rt_led_key_press(treadmill_t *treadmill, const kbd_key_t key);
void rt_led_update(treadmill_t *treadmill);

void rt_led_render(treadmill_t *treadmill);

void rt_led_load_model_params(treadmill_t *treadmill, rt_led_model_t model);

void rt_led_ble_name_update(treadmill_t *treadmill);
int rt_led_send_training_data(treadmill_t *treadmill);

#endif
