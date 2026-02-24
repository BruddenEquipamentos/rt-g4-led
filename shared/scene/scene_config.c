#include "../panel/rt_led.h"
#include "scene.h"

static void on_exit_config_option(treadmill_t *treadmill) {
  switch (treadmill->config.option) {
    case CONFIG_OPTION_BLE_STATE: {
      ble_post_message(
          "\x02"
          "SET,dfu adv=0\n",
          "AOK", 100);
      break;
    }
    default:;
  }
}

static void on_enter_config_option(treadmill_t *treadmill) {
  switch (treadmill->config.option) {
    case CONFIG_OPTION_BLE_STATE: {
      ble_post_message(
          "\x02"
          "SET,dfu adv=1\n",
          "AOK", 500);
      break;
    }
    default:;
  }
}

static void next_config_option(treadmill_t *treadmill) {
  on_exit_config_option(treadmill);
  treadmill->config.option++;
  on_enter_config_option(treadmill);
  if (treadmill->config.option >= CONFIG_OPTION_TOTAL) treadmill->config.option = 0;
}

// exit + set scene here?
static void change_config_model(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT: 
    case KEY_EXT_KBD_SPEED_INC:{
      if (treadmill->config.model + 1 >= RT_LED_MODEL_TOTALS) {
        treadmill->config.model = 0;
      } else {
        treadmill->config.model++;
      }
      rt_led_load_model_params(treadmill, treadmill->config.model);
      treadmill_ble_restart(treadmill);
      break;
    }
    case KEY_SPEED_DECREMENT:
    case KEY_EXT_KBD_SPEED_DEC: {
      if (treadmill->config.model == 0) {
        treadmill->config.model = RT_LED_MODEL_TOTALS - 1;
      } else {
        treadmill->config.model--;
      }
      rt_led_load_model_params(treadmill, treadmill->config.model);
      treadmill_ble_restart(treadmill);
      break;
    }
    default:;
  }
}

// Added to Aria Treadmill - Language = 1: PT-BR, 2: EN-US, 3: ES-ES
static void change_config_language(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT:
    case KEY_EXT_KBD_SPEED_INC: {
      if (treadmill->config.language + 1 >= RT_LED_CONFIG_LNG_TOTALS) {
        treadmill->config.language = 0;
      } else {
        treadmill->config.language++;
      }
      treadmill_ble_restart(treadmill);
      break;
    }
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_DEC:{
      if (treadmill->config.language == 0) {
        treadmill->config.language = RT_LED_CONFIG_LNG_TOTALS - 1;
      } else {
        treadmill->config.language--;
      }
      treadmill_ble_restart(treadmill);
      break;
    }
    default:;
  }
}

static void change_config_weight(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT: 
    case KEY_EXT_KBD_SPEED_INC:{
      if (treadmill->config.weight + 1 <= RT_LED_WEIGHT_MAX)
        treadmill->config.weight++;
      else
        treadmill->config.weight = RT_LED_WEIGHT_MIN;
      break;
    }
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_DEC:{
      if (treadmill->config.weight - 1 >= RT_LED_WEIGHT_MIN)
        treadmill->config.weight--;
      else
        treadmill->config.weight = RT_LED_WEIGHT_MAX;
      break;
    }
    default:;
  }
}

static void change_config_lub(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT: 
    case KEY_EXT_KBD_SPEED_INC:{
      if (treadmill->config.lub_param_km + RT_LED_LUB_STEP_DISTANCE <= RT_LED_LUB_MAX_DISTANCE)
        treadmill->config.lub_param_km += RT_LED_LUB_STEP_DISTANCE;
      else
        treadmill->config.lub_param_km = RT_LED_LUB_MIN_DISTANCE;
      break;
    }
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_DEC:{
      if (treadmill->config.lub_param_km - RT_LED_LUB_STEP_DISTANCE >= RT_LED_LUB_MIN_DISTANCE)
        treadmill->config.lub_param_km -= RT_LED_LUB_MIN_DISTANCE;
      else
        treadmill->config.lub_param_km = RT_LED_LUB_MAX_DISTANCE;
        
      break;
    }
    default:;
  }
}

/////// new config ble name
static void change_config_name(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT: 
    case KEY_EXT_KBD_SPEED_INC: {
      if (treadmill->config.device_name_id + 1 < MAX_DEVICE_NAME_ID){
          treadmill->config.device_name_id += 1;
      }
      else{
          treadmill->config.device_name_id = 1;
      }
      break;
    }
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_DEC:{
      if (treadmill->config.device_name_id - 1 >= 1){
        treadmill->config.device_name_id -= 1;
      }else{
        treadmill->config.device_name_id = 1;
      }
      break;
    }
    default:;
  }
}

static void change_config_modbus(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT:
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_INC:
    case KEY_EXT_KBD_SPEED_DEC:{
      treadmill->config.modbus = treadmill->config.modbus == RT_LED_CONFIG_MODBUS_HIDE_PARAMS
                                     ? RT_LED_CONFIG_MODBUS_SHOW_PARAMS
                                     : RT_LED_CONFIG_MODBUS_HIDE_PARAMS;
      break;
    }
    default:;
  }
}

static void change_config_test(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT:
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_INC:
    case KEY_EXT_KBD_SPEED_DEC:{
      treadmill->config.test =
          treadmill->config.test == RT_LED_CONFIG_TEST_DISABLE ? RT_LED_CONFIG_TEST_ENABLE : RT_LED_CONFIG_TEST_DISABLE;
      break;
    }
    default:;
  }
}

uint8_t config_error_index = 0;

static void change_config_errors(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT:{
      if (config_error_index + 1 < 11) {
        config_error_index++;
      }
      else{
        config_error_index = 0;
      }
      break;
    }
    case KEY_CONFIG:{ // TODO: improve
      treadmill_reset_errors(treadmill);
      break;
    }
    default:;
  }
}

/////// new config MAX TIME
static void change_config_max_time(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT: 
    case KEY_EXT_KBD_SPEED_INC: {
      if (treadmill->config.max_time == 0){
        treadmill->config.max_time = MAX_TIME_ON_TREADMILL_LOWER;
      }
      else{
        if (treadmill->config.max_time + PROGRAM_TIME_INC_TIME_STEP <= DEFAULT_TIME_ON_TREADMILL)
          treadmill->config.max_time += PROGRAM_TIME_INC_TIME_STEP;
        else {
          treadmill->config.max_time = 0;
        }
      }
      break;
    }
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_DEC:{
      if (treadmill->config.max_time - PROGRAM_TIME_INC_TIME_STEP >= MAX_TIME_ON_TREADMILL_LOWER)
        treadmill->config.max_time -= PROGRAM_TIME_INC_TIME_STEP;
      else
        treadmill->config.max_time = 0;
      break;
    }
    default:
      treadmill->config.max_time = 0;
      break;
  }
}

static void change_config_unit(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT: 
    case KEY_EXT_KBD_SPEED_INC:{
      if (treadmill->config.unit + 1 >= RT_LED_CONFIG_UNIT_TOTALS) {
        treadmill->config.unit = RT_LED_CONFIG_UNIT_KM;
      } else {
        treadmill->config.unit++;
      }
      break;
    }
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_DEC:{
      if (treadmill->config.unit == RT_LED_CONFIG_UNIT_KM) {
        treadmill->config.unit = RT_LED_CONFIG_UNIT_MILES;
      } else {
        treadmill->config.unit--;
      }
      break;
    }
    default:;
  }
}

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START: {
      next_config_option(treadmill);
      break;
    }
    case KEY_STOP:
    case KEY_EXT_KBD_STOP: {
      config_error_index = 0;
      treadmill_exit_config(treadmill);
      break;
    }

    case KEY_SPEED_INCREMENT:
    case KEY_SPEED_DECREMENT: 
    case KEY_EXT_KBD_SPEED_INC:
    case KEY_EXT_KBD_SPEED_DEC:{
      switch (treadmill->config.option) {
        case CONFIG_OPTION_MODEL: {
          change_config_model(treadmill, key);
          break;
        }
        case CONFIG_OPTION_LANGUAGE:{
          change_config_language(treadmill, key);
          break;
        }
        case CONFIG_OPTION_UNIT: {
          change_config_unit(treadmill, key);
          break;
        }
        case CONFIG_OPTION_WEIGHT: {
          change_config_weight(treadmill, key);
          break;
        }
        case CONFIG_OPTION_NAME: {
          change_config_name(treadmill, key);
          break;
        }
        case CONFIG_OPTION_MODBUS: {
          change_config_modbus(treadmill, key);
          break;
        }
        case CONFIG_OPTION_TEST: {
          change_config_test(treadmill, key);
          break;
        }
        case CONFIG_OPTION_ERRORS: {
          change_config_errors(treadmill, key);
          break;
        }
        case CONFIG_OPTION_MAX_TIME: {
          change_config_max_time(treadmill, key);
          break;
        }
      }
      break;
    }
    default:;
  }
}

static void update(treadmill_t *treadmill) {}

const scene_t scene_config = {
    .id = SCENE_CONFIG,
    .name = "CONFIG",
    .keypress = keypress,
    .update = update,
};
