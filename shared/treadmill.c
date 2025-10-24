#include "treadmill.h"

#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "nvm/nvm.h"
#include "panel/rt_led.h"

#include "ble/ble_protocol/ble_ftms_treadmill.h"
#include "ble/ble_protocol/ble_protocol_helper.h" //Lucas 23/03

#include "lang.h"

char *scene_name[SCENE_TOTALS] = {
    "SCENE_INIT",    "SCENE_IDLE",      "SCENE_EMERGENCY", "SCENE_LUB",    "SCENE_CONFIG",
    "SCENE_PROGRAM", "SCENE_COUNTDOWN", "SCENE_TRAINING",  "SCENE_PAUSED", "SCENE_REPORT", "SCENE_BLE_ALLOW_Y_N",
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
  if (pTreadmill->config.unit == RT_LED_CONFIG_UNIT_MILES){ //imperial
    unsigned long long speed_mph = (((unsigned long long)speed) * 621371) / ((unsigned long long)1000000); // add 0
    return ((600000000.0 / speed_mph) * 6) / 10000;
  }
  else{ //metric
    return ((60000000.0 / speed) * 6) / 1000;
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
    uint32_t speed_mod = speed % 10;
    // if (speed_mod >= 5)
    //     speed = speed + (10 - speed_mod);
    return speed;
}

uint32_t treadmill_calculate_calories(treadmill_t *treadmill) {
  // No painel de LED atual, não se leva em consideração o BPM
  // no calculo de gasto calórico.
  // No painel de LED atual, não há diferença no cálculo de
  // gasto calórico entre homem e mulher
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

  if (nvm.NVM_ReadString(NVM_NSP_START_ADDRESS, (uint8_t *)ns, 22)) {
    strncpy(treadmill->config.nsp, ns, 22);
  }

  if (nvm.NVM_ReadString(NVM_NSI_START_ADDRESS, (uint8_t *)ns, 22)) {
    strncpy(treadmill->config.nsi, ns, 22);
  }

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
  treadmill_ble_restart(treadmill);
}

void treadmill_update(treadmill_t *treadmill) {
  if (treadmill->message.tick && treadmill->get_time_ms() - treadmill->message.tick >= treadmill->message.duration) {
    treadmill->message.tick = 0;
    treadmill->message.duration = 0;
  }

  rt_led_update(treadmill);
}

void treadmill_emergency_pressed(treadmill_t *treadmill) {
  uint8_t buffer[64];
  uint8_t length;
  build_hrm_disconnect_packet(buffer, &length, 64);
  ble_post_message(buffer, "", 0);
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
