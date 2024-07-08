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
    case KEY_SPEED_INCREMENT: {
      if (treadmill->config.model + 1 >= RT_LED_MODEL_TOTALS) {
        treadmill->config.model = 0;
      } else {
        treadmill->config.model++;
      }
      rt_led_load_model_params(treadmill, treadmill->config.model);
      treadmill_ble_restart(treadmill);
      break;
    }
    case KEY_SPEED_DECREMENT: {
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

static void change_config_weight(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT: {
      if (treadmill->config.weight + 1 <= RT_LED_WEIGHT_MAX)
        treadmill->config.weight++;
      else
        treadmill->config.weight = RT_LED_WEIGHT_MIN;
      break;
    }
    case KEY_SPEED_DECREMENT: {
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
    case KEY_SPEED_INCREMENT: {
      if (treadmill->config.lub_param_km + RT_LED_LUB_STEP_DISTANCE <= RT_LED_LUB_MAX_DISTANCE)
        treadmill->config.lub_param_km += RT_LED_LUB_STEP_DISTANCE;
      else
        treadmill->config.lub_param_km = RT_LED_LUB_MIN_DISTANCE;
      break;
    }
    case KEY_SPEED_DECREMENT: {
      if (treadmill->config.lub_param_km - RT_LED_LUB_STEP_DISTANCE >= RT_LED_LUB_MIN_DISTANCE)
        treadmill->config.lub_param_km -= RT_LED_LUB_MIN_DISTANCE;
      else
        treadmill->config.lub_param_km = RT_LED_LUB_MAX_DISTANCE;

      break;
    }
    default:;
  }
}

static void change_config_modbus(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_INCREMENT:
    case KEY_SPEED_DECREMENT: {
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
    case KEY_SPEED_DECREMENT: {
      treadmill->config.test =
          treadmill->config.test == RT_LED_CONFIG_TEST_DISABLE ? RT_LED_CONFIG_TEST_ENABLE : RT_LED_CONFIG_TEST_DISABLE;
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
    case KEY_EXT_KBD_STOP: {
      treadmill_exit_config(treadmill);
      break;
    }

    case KEY_SPEED_INCREMENT:
    case KEY_SPEED_DECREMENT: {
      switch (treadmill->config.option) {
        case CONFIG_OPTION_MODEL: {
          change_config_model(treadmill, key);
          break;
        }
        case CONFIG_OPTION_WEIGHT: {
          change_config_weight(treadmill, key);
          break;
        }
        case CONFIG_OPTION_LUB: {
          change_config_lub(treadmill, key);
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
