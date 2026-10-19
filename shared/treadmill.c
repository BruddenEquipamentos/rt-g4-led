#include "treadmill.h"

#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "nvm/nvm.h"
#include "panel/rt_led.h"
#include "./dm/dm_render.h"
#include "./dm/dm.h"

char *scene_name[SCENE_TOTALS] = {
    "SCENE_INIT",    "SCENE_IDLE",      "SCENE_EMERGENCY", "SCENE_LUB",    "SCENE_CONFIG",
    "SCENE_PROGRAM", "SCENE_COUNTDOWN", "SCENE_TRAINING",  "SCENE_PAUSED", "SCENE_REPORT",
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
}

uint32_t treadmill_calculate_pace(treadmill_t *treadmill) {
  return ((60000000 / treadmill->training.speed) * 6) / 1000;
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

  uint32_t lub_param_in_km = 0;
  if (nvm.NVM_ReadVariable32bits(NVM_U32_LUB_DISTANCE_PARAM, &lub_param_in_km)) {
    treadmill->config.lub_param_km = lub_param_in_km;
  } else {
    treadmill->config.lub_param_km = DEFAULT_LUB_DISTANCE_PARAM;
  }

  uint32_t lub_distance_in_m = 0;
  if (nvm.NVM_ReadVariable32bits(NVM_U32_LUB_DISTANCE, &lub_distance_in_m)) {
    treadmill->config.lub_distance_m = lub_distance_in_m;
  } else {
    treadmill->config.lub_distance_m = treadmill->total.distance_in_meters;
  }

  uint8_t age_in_years = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_AGE, &age_in_years)) {
    treadmill->config.age = age_in_years;
  } else {
    treadmill->config.age = DEFAULT_AGE;
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
  if (treadmill->previous_config.lub_param_km != treadmill->config.lub_param_km) {
    nvm.NVM_WriteVariable32bits(NVM_U32_LUB_DISTANCE_PARAM, treadmill->config.lub_param_km);
  }
  if (treadmill->previous_config.age != treadmill->config.age) {
    nvm.NVM_WriteVariable8bits(NVM_U8_AGE, treadmill->config.age);
  }
}

void treadmill_update(treadmill_t *treadmill) {
  if (treadmill->message.tick && treadmill->get_time_ms() - treadmill->message.tick >= treadmill->message.duration) {
    treadmill->message.tick = 0;
    treadmill->message.duration = 0;
  }

  rt_led_update(treadmill);
}

void treadmill_emergency_pressed(treadmill_t *treadmill) {
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
  dm_reset_frame(); //
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

void treadmill_ble_restart(treadmill_t *treadmill) { treadmill->hw.ble.init_state = BLE_INIT_STATE_INIT; }
