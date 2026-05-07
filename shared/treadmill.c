#include "treadmill.h"

#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "nvm/nvm.h"
#include "panel/rt_led.h"

#include "ble/ble_protocol/ble_ftms_treadmill.h"
#include "ble/ble_protocol/ble_protocol_command.h"
#include "ble/ble_protocol/ble_protocol_helper.h" //Lucas 23/03

#include "lang.h"

char *scene_name[SCENE_TOTALS] = {
    "SCENE_INIT",    "SCENE_IDLE",      "SCENE_EMERGENCY", "SCENE_LUB",    "SCENE_CONFIG",
    "SCENE_PROGRAM", "SCENE_COUNTDOWN", "SCENE_TRAINING",  "SCENE_PAUSED", "SCENE_REPORT", "SCENE_BLE_ALLOW_Y_N", "SCENE_TEST"
};

__attribute__((weak)) void beep(beep_t beep) { (void)beep; }
__attribute__((weak)) void update_rtc(const uint32_t ms) { (void)ms; }

__attribute__((weak)) int motor_send(const uint8_t *data, int length) {
  (void)data;
  (void)length;
  return 1;
}
__attribute__((weak)) int motor_recv(uint8_t *data, int max_length) {
  (void)data;
  (void)max_length;
  return 0;
}

#define ERROR_SAVE_DEBOUNCE_MS 5000u

static int keyboard_error_code_to_index(keyboard_error_code_t code) {
  switch (code) {
    case KEYBOARD_ERROR_MAIN_STUCK:
      return 0;
    case KEYBOARD_ERROR_MAIN_BURST:
      return 1;
    case KEYBOARD_ERROR_HG_SPEED_STUCK:
      return 2;
    case KEYBOARD_ERROR_HG_SPEED_BURST:
      return 3;
    case KEYBOARD_ERROR_HG_START_STOP_STUCK:
      return 4;
    case KEYBOARD_ERROR_HG_START_STOP_BURST:
      return 5;
    default:
      return -1;
  }
}

static uint8_t keyboard_error_code_to_source_mask(keyboard_error_code_t code) {
  switch (code) {
    case KEYBOARD_ERROR_MAIN_STUCK:
    case KEYBOARD_ERROR_MAIN_BURST:
      return KEYBOARD_SOURCE_MASK_MAIN;
    case KEYBOARD_ERROR_HG_SPEED_STUCK:
    case KEYBOARD_ERROR_HG_SPEED_BURST:
      return KEYBOARD_SOURCE_MASK_HG_SPEED;
    case KEYBOARD_ERROR_HG_START_STOP_STUCK:
    case KEYBOARD_ERROR_HG_START_STOP_BURST:
      return KEYBOARD_SOURCE_MASK_HG_START_STOP;
    default:
      return 0;
  }
}

void treadmill_setup(treadmill_t *treadmill) {
  memset(treadmill, 0x00, sizeof(treadmill_t));
  strncpy(treadmill->config.device_name, "MovCT", 15);

  treadmill->hw.beep = beep;
  treadmill->hw.update_rtc = update_rtc;
  treadmill->hw.inverter.send_data = motor_send;
  treadmill->hw.inverter.recv_data = motor_recv;

  treadmill->scene = SCENE_INIT;
  treadmill->scene_tick = 0;
  treadmill_load_config(treadmill);
  treadmill_ble_restart(treadmill); //Lucas 28/03
  treadmill->debug.print_states = true;
}

void treadmill_set_scene(treadmill_t *treadmill, treadmill_scene_t scene) {
  treadmill->previous_scene = treadmill->scene;
  treadmill->scene = scene;
  treadmill->scene_tick = treadmill->get_time_ms();
  treadmill->idle_tick = treadmill->get_time_ms();
  
  // if (treadmill->on_state_changed_callback) {
  //   treadmill->on_state_changed_callback(treadmill, scene);
  // }
// switch (scene) {
//         case SCENE_TRAINING: {
//             ble_post_message(
//             "\x02"
//             "SET_BLE_FLAGS,32\n",
//             "AOK", 1000);
//             break;
//         }
//         case SCENE_PAUSED: {
//             ble_post_message(
//             "\x02"
//             "SET_BLE_FLAGS,8\n",
//             "AOK", 1000);
//             break;
//         }
//         case SCENE_EMERGENCY: {
//             ble_post_message(
//             "\x02"
//             "SET_BLE_FLAGS,128\n", // check this later
//             "AOK", 1000);
//             break;
//         }
//         case SCENE_BLE_ALLOW_YES_NO: {
//             if (treadmill->hw.ble.connected == 1){
//               ble_post_message(
//               "\x02"
//               "SET_BLE_FLAGS,4\n",
//               "AOK", 1000);
//             }
//             else{}
//             break;
//         }
//         default: {
//             ble_post_message(
//             "\x02"
//             "SET_BLE_FLAGS,16\n",
//             "AOK", 1000);
//             break;
//         }
//     }

}
uint32_t treadmill_calculate_pace(const uint16_t speed) {
  extern treadmill_t *pTreadmill;
  if (speed == 0) return 0;

  if (pTreadmill->config.unit == RT_LED_CONFIG_UNIT_MILES){ //imperial
    // speed is in km/h x100. Convert to mph x100 with 32-bit integer arithmetic.
    uint32_t speed_mph = ((uint32_t)speed * 62137u) / 100000u;
    if (speed_mph == 0) return 0;
    return 360000u / speed_mph;
  }
  else{ //metric
    return 360000u / speed;
  }
}

static int walking_speed(const uint16_t speed) { return speed < 600; }

volatile uint32_t _kcal = 0;
// volatile float _fKcal = 0.0f;
//
// volatile float exp1 = 0;
// volatile float exp2 = 0;

uint32_t _exp1 = 0;
uint32_t _exp2 = 0;

uint32_t _result;

uint32_t treadmill_current_average_speed(treadmill_t *treadmill) {
    if (treadmill->training.seconds == 0)
        return 0;

    uint32_t cm_sec = (treadmill->training.meters * 100 / treadmill->training.seconds);
    uint32_t speed = ((cm_sec * 36) / 10); // 333 * 36 = 1198
    // if (speed_mod >= 5)
    //     speed = speed + (10 - speed_mod);
    return speed;
}

uint32_t treadmill_calculate_calories(treadmill_t *treadmill) {
  // No painel de LED atual, nÃ£o se leva em consideraÃ§Ã£o o BPM
  // no calculo de gasto calÃ³rico.
  // No painel de LED atual, nÃ£o hÃ¡ diferenÃ§a no cÃ¡lculo de
  // gasto calÃ³rico entre homem e mulher
  // float fAge = treadmill->config.age;

  uint32_t factor = 1;
  uint32_t speed = treadmill->training.speed * factor;
  uint32_t incline = treadmill->training.incline * (factor / 10);
  uint32_t weight = treadmill->config.weight;
  uint32_t time = 1;
  uint32_t kcal = 0;

  if (walking_speed(treadmill->training.speed)) {
    _exp1 = ((speed * 100 / 6) * (10 + 180 * incline / 100) + 350 * factor * 100);
    _exp2 = (5 * weight * time * factor * factor);
    //  fKcal = (((fSpeed / 0.06) * (0.1 + 1.8 * (fIncline / 100)) + 3.5) * (5 * fWeight * fTime)) / 1000;
  } else {
    _exp1 = ((speed * 100 / 6) * (20 + 90 * incline / 100) + 350 * factor * 100);
    _exp2 = (5 * weight * time * factor * factor);
    // fKcal = (((fSpeed / 0.06) * (0.2 + 0.9 * (fIncline / 100)) + 3.5) * (5 * fWeight * fTime)) / 1000;
  }

  kcal = (_exp1 * _exp2);

  return kcal / 600000;
}

void treadmill_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  if (treadmill->key_pressed != key) {
    treadmill->key_pressed_tick = treadmill->get_time_ms();
    treadmill->key_pressed_last_tick = treadmill->get_time_ms();
    treadmill->key_pressed = key;
  }
  rt_led_key_press(treadmill, key);
}

void treadmill_key_press_clear(treadmill_t *treadmill) {
  treadmill->key_pressed = KEY_NONE;
  treadmill->key_pressed_tick = 0;
}

void treadmill_load_config(treadmill_t *treadmill) {
  rt_led_model_t model = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_MODEL, (uint8_t *)&model)) {
    rt_led_load_model_params(treadmill, model);
  } else {
    rt_led_load_model_params(treadmill, RT_LED_MODEL_250_USART);
    treadmill_save_config(treadmill);
  }

  uint32_t total_distance_in_meters = 0;
  if (nvm.NVM_ReadVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS, &total_distance_in_meters)) {
    treadmill->total.distance_in_meters = total_distance_in_meters;
  } else {
    treadmill->total.distance_in_meters = 0;
  }

  uint32_t total_time_in_seconds = 0;
  if (nvm.NVM_ReadVariable32bits(NVM_U32_TOTAL_TIME_SECONDS, &total_time_in_seconds)) {
    treadmill->total.time_in_seconds = total_time_in_seconds;
  } else {
    treadmill->total.time_in_seconds = 0;
  }

  uint8_t weight_in_kg = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_WEIGHT, &weight_in_kg)) {
    treadmill->config.weight = weight_in_kg;
  } else {
    treadmill->config.weight = DEFAULT_WEIGHT_IN_KG;
  }

  // uint32_t lub_param_in_km = 0;
  // if (nvm.NVM_ReadVariable32bits(NVM_U32_LUB_DISTANCE_PARAM, &lub_param_in_km)) {
  //   treadmill->config.lub_param_km = lub_param_in_km;
  // } else {
  //   treadmill->config.lub_param_km = DEFAULT_LUB_DISTANCE_PARAM;
  // }

  // uint32_t lub_distance_in_m = 0;
  // if (nvm.NVM_ReadVariable32bits(NVM_U32_LUB_DISTANCE, &lub_distance_in_m)) {
  //   treadmill->config.lub_distance_m = lub_distance_in_m;
  // } else {
  //   treadmill->config.lub_distance_m = treadmill->total.distance_in_meters;
  // }

  uint8_t age_in_years = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_AGE, &age_in_years)) {
    treadmill->config.age = age_in_years;
  } else {
    treadmill->config.age = DEFAULT_AGE;
  }

  uint32_t max_time_on_treadmill = 0;
  if (nvm.NVM_ReadVariable32bits(NVM_U32_MAX_TIME_PARAM, &max_time_on_treadmill)) {
    treadmill->config.max_time = max_time_on_treadmill;
  } else {
    treadmill->config.max_time = 0;
  } /////// max time config

  uint8_t c2v = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_C2V, &c2v)) {
    treadmill->config.c2v = (c2v != 0) ? RT_LED_CONFIG_C2V_ENABLE : RT_LED_CONFIG_C2V_DISABLE;
  } else {
    treadmill->config.c2v = RT_LED_CONFIG_C2V_DISABLE;
    nvm.NVM_WriteVariable8bits(NVM_U8_C2V, (uint8_t)treadmill->config.c2v);
  }

  uint8_t language = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_LANG, &language)) {
    treadmill->config.language = language;
  } else {
    treadmill->config.language = RT_LED_CONFIG_LNG_PT_BR;
  }

  uint8_t unit = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_UNIT, &unit)) {
    treadmill->config.unit = unit;
  } else {
    treadmill->config.unit = RT_LED_CONFIG_UNIT_KM;
  }

  uint8_t name_id = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_NAME, &name_id)) {
    treadmill->config.device_name_id = name_id;
    rt_led_ble_name_update(treadmill);
  } else {
    treadmill->config.device_name_id = 1; //default name id
    rt_led_ble_name_update(treadmill);
  }

  char ns[22];
  if (nvm.NVM_ReadString(NVM_NSU_START_ADDRESS, (uint8_t *)ns, 22)) {
    strncpy(treadmill->config.nsu, ns, 22);
  }
  // testing
  // for (int i = 0; i < 22; ++i) {
  //   uint8_t b;
  //   if (nvm.NVM_ReadVariable8bits(NVM_NSU_START_ADDRESS + i, &b)
  //           == NVM_STATUS_OK) {
  //       treadmill->config.nsu[i] = b;
  //   } else {
  //       treadmill->config.nsu[i] = '\0';
  //       break;
  //   }
  // }


  if (nvm.NVM_ReadString(NVM_NSP_START_ADDRESS, (uint8_t *)ns, 22)) {
    strncpy(treadmill->config.nsp, ns, 22);
  }

  if (nvm.NVM_ReadString(NVM_NSI_START_ADDRESS, (uint8_t *)ns, 22)) {
    strncpy(treadmill->config.nsi, ns, 22);
  }

  treadmill_load_errors(treadmill);

  DPRINT("nsu:%s\n", treadmill->config.nsu);
  DPRINT("nsp:%s\n", treadmill->config.nsp);
  DPRINT("nsi:%s\n", treadmill->config.nsi);
}

void treadmill_save_config(treadmill_t *treadmill) {
  if (treadmill->previous_config.model != treadmill->config.model) {
    nvm.NVM_WriteVariable8bits(NVM_U8_MODEL, treadmill->config.model);
  }
  if (treadmill->previous_config.weight != treadmill->config.weight) {
    nvm.NVM_WriteVariable8bits(NVM_U8_WEIGHT, treadmill->config.weight);
  }
  if (treadmill->previous_config.language != treadmill->config.language) {
    nvm.NVM_WriteVariable8bits(NVM_U8_LANG, treadmill->config.language);
  }
  if (treadmill->previous_config.unit != treadmill->config.unit) {
    nvm.NVM_WriteVariable8bits(NVM_U8_UNIT, treadmill->config.unit);
  }

  if (treadmill->previous_config.device_name_id != treadmill->config.device_name_id) {
    nvm.NVM_WriteVariable8bits(NVM_U8_NAME, treadmill->config.device_name_id);
    rt_led_ble_name_update(treadmill);
  }
  // if (treadmill->previous_config.lub_param_km != treadmill->config.lub_param_km) {
  //   nvm.NVM_WriteVariable32bits(NVM_U32_LUB_DISTANCE_PARAM, treadmill->config.lub_param_km);
  // }
  if (treadmill->previous_config.age != treadmill->config.age) {
    nvm.NVM_WriteVariable8bits(NVM_U8_AGE, treadmill->config.age);
  }
  if (treadmill->previous_config.max_time != treadmill->config.max_time) {  ////// config max time
    nvm.NVM_WriteVariable32bits(NVM_U32_MAX_TIME_PARAM, treadmill->config.max_time);
  }
  if (treadmill->previous_config.c2v != treadmill->config.c2v) {
    nvm.NVM_WriteVariable8bits(NVM_U8_C2V, (uint8_t)treadmill->config.c2v);
  }
  treadmill_ble_restart(treadmill);
}

void treadmill_update(treadmill_t *treadmill) {
  if (treadmill->message.tick && treadmill->get_time_ms() - treadmill->message.tick >= treadmill->message.duration) {
    treadmill->message.tick = 0;
    treadmill->message.duration = 0;
  }

  uint32_t now = treadmill->get_time_ms();
  int should_save_errors =
      (treadmill->hw.inverter.errors_dirty &&
       (now - treadmill->hw.inverter.last_error_save_time_ms) >= ERROR_SAVE_DEBOUNCE_MS) ||
      (treadmill->hw.keyboard.errors_dirty &&
       (now - treadmill->hw.keyboard.last_error_save_time_ms) >= ERROR_SAVE_DEBOUNCE_MS);
  if (should_save_errors) {
    treadmill_save_errors(treadmill);
    if (treadmill->hw.inverter.errors_dirty) {
      treadmill->hw.inverter.last_error_save_time_ms = now;
      treadmill->hw.inverter.errors_dirty = 0;
    }
    if (treadmill->hw.keyboard.errors_dirty) {
      treadmill->hw.keyboard.last_error_save_time_ms = now;
      treadmill->hw.keyboard.errors_dirty = 0;
    }
  }

  rt_led_update(treadmill);
}

void treadmill_emergency_pressed(treadmill_t *treadmill) {
  uint8_t buffer[64];
  uint8_t length;
  build_hrm_disconnect_packet(buffer, &length, 64);
  ble_post_message((const char *)buffer, "", 0);
  if (treadmill->on_update_fms_callback) { //Lucas 23/03
    uint8_t stopped = 0x01;
    treadmill->on_update_fms_callback(treadmill, StoppedOrPausedByTheUser, &stopped);
  }
  if (treadmill->scene != SCENE_EMERGENCY) {
    treadmill_reset_training(treadmill);
    treadmill_set_scene(treadmill, SCENE_EMERGENCY);
    treadmill->hw.beep(BEEP_EMERGENCY);
  }
}

void treadmill_set_dm_message(treadmill_t *treadmill, const char *text, const uint32_t duration_ms,
                              const uint32_t blink_interval) {
  strncpy(treadmill->message.data, text, 32);
  treadmill->message.tick = treadmill->get_time_ms();
  treadmill->message.duration = duration_ms;
  treadmill->message.blink_interval = blink_interval;
}

void treadmill_clear_dm_message(treadmill_t *treadmill) {
  treadmill->message.blink_interval = 0;
  treadmill->message.tick = 0;
}

/*
lub_param_m
lub_distance_m
*/

uint32_t treadmill_calculate_lub_distance_remaining(treadmill_t *treadmill) {
  uint32_t param = treadmill->config.lub_param_km * 1000;
  uint32_t remaining = 0;
  uint32_t distance = 0;
  if (treadmill->total.distance_in_meters >= treadmill->config.lub_distance_m)
    distance = treadmill->total.distance_in_meters - treadmill->config.lub_distance_m;
  if (param >= distance) remaining = (param - distance) / 1000;
  return remaining;
}

void treadmill_reset_lub(treadmill_t *treadmill) {
  if (nvm.NVM_WriteVariable32bits(NVM_U32_LUB_DISTANCE, treadmill->total.distance_in_meters) == NVM_STATUS_OK) {
    treadmill->config.lub_distance_m = treadmill->total.distance_in_meters;
  }
}

const char *get_string(treadmill_t *treadmill, int str_id) {
  if (str_id < 0 || str_id >= STR_TOTALS)
      return "INVALID STRING ID";

  const char *str = strings[treadmill->config.language][str_id];
  if (!str || str[0] == '\0')  // fallback se string vazia ou nula
      str = strings[RT_LED_CONFIG_LNG_PT_BR][str_id];

  return str;
}

void treadmill_ble_restart(treadmill_t *treadmill) { 
  treadmill->hw.ble.state = BLE_STATE_INIT; 
  treadmill->hw.ble.init_state = BLE_INIT_STATE_INIT;
}

void treadmill_register_keyboard_error(treadmill_t *treadmill, keyboard_error_code_t code) {
  if (treadmill == NULL || code == KEYBOARD_ERROR_NONE) return;
  if (treadmill->scene == SCENE_CONFIG) return;

  int idx = keyboard_error_code_to_index(code);
  if (idx < 0) return;

  struct keyboard *kbd = &treadmill->hw.keyboard;
  uint32_t now = treadmill->get_time_ms();

  kbd->last_errors[2] = kbd->last_errors[1];
  kbd->last_errors[1] = kbd->last_errors[0];
  kbd->last_errors[0] = code;
  kbd->previous_error_code = (uint8_t)code;

  kbd->error[idx].code = code;
  if (kbd->error[idx].count < 0xffffu) {
    kbd->error[idx].count++;
  }
  kbd->error[idx].error_time_ms = now;

  if ((now - kbd->last_error_save_time_ms) >= ERROR_SAVE_DEBOUNCE_MS) {
    treadmill_save_errors(treadmill);
    kbd->last_error_save_time_ms = now;
    kbd->errors_dirty = 0;
  } else {
    kbd->errors_dirty = 1;
  }
}

uint8_t treadmill_get_latest_error_code(treadmill_t *treadmill) {
  uint8_t latest_code = 0;
  uint32_t latest_time = 0;

  for (uint8_t i = 0; i < INVERTER_ERROR_COUNT; i++) {
    if (treadmill->hw.inverter.error[i].error_time_ms > latest_time) {
      latest_time = treadmill->hw.inverter.error[i].error_time_ms;
      latest_code = (uint8_t)treadmill->hw.inverter.error[i].code;
    }
  }

  for (uint8_t i = 0; i < KEYBOARD_ERROR_COUNT; i++) {
    if (treadmill->hw.keyboard.error[i].error_time_ms > latest_time) {
      latest_time = treadmill->hw.keyboard.error[i].error_time_ms;
      latest_code = (uint8_t)treadmill->hw.keyboard.error[i].code;
    }
  }

  if (latest_time == 0) {
    if (treadmill->hw.keyboard.last_errors[0] != KEYBOARD_ERROR_NONE) {
      return (uint8_t)treadmill->hw.keyboard.last_errors[0];
    }
    return (uint8_t)treadmill->hw.inverter.last_errors[0];
  }

  return latest_code;
}

void treadmill_save_errors(treadmill_t *treadmill) {
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_0_COUNT, treadmill->hw.inverter.error[0].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_1_COUNT, treadmill->hw.inverter.error[1].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_2_COUNT, treadmill->hw.inverter.error[2].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_3_COUNT, treadmill->hw.inverter.error[3].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_4_COUNT, treadmill->hw.inverter.error[4].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_5_COUNT, treadmill->hw.inverter.error[5].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_6_COUNT, treadmill->hw.inverter.error[6].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_7_COUNT, treadmill->hw.inverter.error[7].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_8_COUNT, treadmill->hw.inverter.error[8].count);
  nvm.NVM_WriteVariable8bits(NVM_U8_INVERTER_LAST_ERROR_0, treadmill->hw.inverter.last_errors[0]);
  nvm.NVM_WriteVariable8bits(NVM_U8_INVERTER_LAST_ERROR_1, treadmill->hw.inverter.last_errors[1]);
  nvm.NVM_WriteVariable8bits(NVM_U8_INVERTER_LAST_ERROR_2, treadmill->hw.inverter.last_errors[2]);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_0_COUNT, treadmill->hw.keyboard.error[0].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_1_COUNT, treadmill->hw.keyboard.error[1].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_2_COUNT, treadmill->hw.keyboard.error[2].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_3_COUNT, treadmill->hw.keyboard.error[3].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_4_COUNT, treadmill->hw.keyboard.error[4].count);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_5_COUNT, treadmill->hw.keyboard.error[5].count);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_0, treadmill->hw.keyboard.last_errors[0]);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_1, treadmill->hw.keyboard.last_errors[1]);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_2, treadmill->hw.keyboard.last_errors[2]);
}

void treadmill_reset_errors(treadmill_t *treadmill) {
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_0_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_1_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_2_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_3_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_4_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_5_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_6_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_7_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_INVERTER_ERROR_8_COUNT, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_INVERTER_LAST_ERROR_0, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_INVERTER_LAST_ERROR_1, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_INVERTER_LAST_ERROR_2, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_0_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_1_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_2_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_3_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_4_COUNT, 0);
  nvm.NVM_WriteVariable16bits(NVM_U16_KEYBOARD_ERROR_5_COUNT, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_0, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_1, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_2, 0);

  for (uint8_t i = 0; i < INVERTER_ERROR_COUNT; i++) {
    treadmill->hw.inverter.error[i].count = 0;
    treadmill->hw.inverter.error[i].error_time_ms = 0;
  }
  for (uint8_t i = 0; i < LAST_ERROR_HISTORY_COUNT; i++) {
    treadmill->hw.inverter.last_errors[i] = INVERTER_ERROR_NONE;
  }
  for (uint8_t i = 0; i < KEYBOARD_ERROR_COUNT; i++) {
    treadmill->hw.keyboard.error[i].count = 0;
    treadmill->hw.keyboard.error[i].error_time_ms = 0;
  }
  for (uint8_t i = 0; i < LAST_ERROR_HISTORY_COUNT; i++) {
    treadmill->hw.keyboard.last_errors[i] = KEYBOARD_ERROR_NONE;
  }
  treadmill->hw.inverter.errors_dirty = 0;
  treadmill->hw.keyboard.errors_dirty = 0;
  treadmill->hw.inverter.last_error_save_time_ms = 0;
  treadmill->hw.keyboard.last_error_save_time_ms = 0;
  treadmill->hw.keyboard.disabled_sources_mask = 0;
}

void treadmill_clear_keyboard_error_code_and_unlock(treadmill_t *treadmill) {
  if (treadmill == NULL) return;

  for (uint8_t i = 0; i < LAST_ERROR_HISTORY_COUNT; i++) {
    treadmill->hw.keyboard.last_errors[i] = KEYBOARD_ERROR_NONE;
  }

  for (uint8_t i = 0; i < KEYBOARD_ERROR_COUNT; i++) {
    treadmill->hw.keyboard.error[i].error_time_ms = 0;
  }

  treadmill->hw.keyboard.previous_error_code = (uint8_t)KEYBOARD_ERROR_NONE;
  treadmill->hw.keyboard.disabled_sources_mask = 0;
  treadmill->hw.keyboard.errors_dirty = 0;
  treadmill->hw.keyboard.last_error_save_time_ms = 0;

  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_0, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_1, 0);
  nvm.NVM_WriteVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_2, 0);
}

void treadmill_load_errors(treadmill_t *treadmill) {
  uint16_t error_count = 0;

  // Carregar contadores de erro
  for (uint8_t i = 0; i < INVERTER_ERROR_COUNT; i++) {
    if (nvm.NVM_ReadVariable16bits(NVM_U16_INVERTER_ERROR_0_COUNT + i, &error_count)) {
      treadmill->hw.inverter.error[i].count = error_count;
    } else {
      treadmill->hw.inverter.error[i].count = 0;
    }
    treadmill->hw.inverter.error[i].error_time_ms = 0;
  }

  uint8_t last_error = 0;

  // Carregar ultimos 3 erros
  for (uint8_t i = 0; i < LAST_ERROR_HISTORY_COUNT; i++) {
    if (nvm.NVM_ReadVariable8bits(NVM_U8_INVERTER_LAST_ERROR_0 + i, &last_error)) {
      treadmill->hw.inverter.last_errors[i] = last_error;
    } else {
      treadmill->hw.inverter.last_errors[i] = INVERTER_ERROR_NONE;
    }
  }

  for (uint8_t i = 0; i < KEYBOARD_ERROR_COUNT; i++) {
    if (nvm.NVM_ReadVariable16bits(NVM_U16_KEYBOARD_ERROR_0_COUNT + i, &error_count)) {
      treadmill->hw.keyboard.error[i].count = error_count;
    } else {
      treadmill->hw.keyboard.error[i].count = 0;
    }
    treadmill->hw.keyboard.error[i].error_time_ms = 0;
  }

  for (uint8_t i = 0; i < LAST_ERROR_HISTORY_COUNT; i++) {
    if (nvm.NVM_ReadVariable8bits(NVM_U8_KEYBOARD_LAST_ERROR_0 + i, &last_error)) {
      treadmill->hw.keyboard.last_errors[i] = last_error;
    } else {
      treadmill->hw.keyboard.last_errors[i] = KEYBOARD_ERROR_NONE;
    }
  }

  treadmill->hw.inverter.previous_error_code = (uint8_t)treadmill->hw.inverter.last_errors[0];
  treadmill->hw.keyboard.previous_error_code = (uint8_t)treadmill->hw.keyboard.last_errors[0];
  treadmill->hw.inverter.errors_dirty = 0;
  treadmill->hw.keyboard.errors_dirty = 0;
  treadmill->hw.inverter.last_error_save_time_ms = 0;
  treadmill->hw.keyboard.last_error_save_time_ms = 0;
  uint8_t disabled_sources_mask = 0;
  for (uint8_t i = 0; i < LAST_ERROR_HISTORY_COUNT; i++) {
    disabled_sources_mask |= keyboard_error_code_to_source_mask(treadmill->hw.keyboard.last_errors[i]);
  }
  treadmill->hw.keyboard.disabled_sources_mask = disabled_sources_mask;
}
