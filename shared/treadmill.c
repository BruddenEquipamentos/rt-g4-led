#include "treadmill.h"

#include <stdbool.h>
#include <string.h>

#include "nvm/nvm.h"
#include "panel/rt_led.h"

char *scene_name[SCENE_TOTALS] = {
    "SCENE_INIT",     "SCENE_IDLE",    "SCENE_LUB",
    "SCENE_CONFIG",   "SCENE_PROGRAM", "SCENE_COUNTDOWN",
    "SCENE_TRAINING", "SCENE_PAUSED",  "SCENE_REPORT",
};

__attribute__((weak)) void beep(beep_t beep) { (void)beep; }

void treadmill_setup(treadmill_t *treadmill) {
  memset(treadmill, 0x00, sizeof(treadmill_t));
  treadmill->beep = beep;
  treadmill->scene = SCENE_INIT;
  treadmill->scene_tick = 0;

  treadmill_load_config(treadmill);

  treadmill->debug.print_states = true;
}

void treadmill_set_scene(treadmill_t *treadmill, treadmill_scene_t scene) {
  treadmill->scene = scene;
  treadmill->scene_tick = treadmill->get_time_ms();
}

uint32_t treadmill_calculate_pace(treadmill_t *treadmill) {
  return ((60000000 / treadmill->training.speed) * 6) / 1000;
}

void treadmill_key_press(treadmill_t *treadmill, const key_t key) {
  rt_led_key_press(treadmill, key);
}

void treadmill_load_config(treadmill_t *treadmill) {
  rt_led_model_t model = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_MODEL, (uint8_t *)&model)) {
    rt_led_load_model_params(treadmill, model);
  } else {
    rt_led_load_model_params(treadmill, RT_LED_MODEL_150_USART);
    treadmill_save_config(treadmill);
  }

  uint32_t total_distance_in_meters = 0;
  if (nvm.NVM_ReadVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS,
                                 &total_distance_in_meters)) {
    treadmill->total.distance_in_meters = total_distance_in_meters;
  }

  uint32_t total_time_in_seconds = 0;
  if (nvm.NVM_ReadVariable32bits(NVM_U32_TOTAL_TIME_MINUTES,
                                 &total_time_in_seconds)) {
    treadmill->total.time_in_seconds = total_time_in_seconds;
  }

  uint8_t weight_in_kg = 0;
  if (nvm.NVM_ReadVariable8bits(NVM_U8_WEIGHT, &weight_in_kg)) {
    treadmill->config.weight = weight_in_kg;
  }
}

void treadmill_save_config(treadmill_t *treadmill) {
  if (treadmill->previous_config.model != treadmill->config.model) {
    nvm.NVM_WriteVariable8bits(NVM_U8_MODEL, treadmill->config.model);
  }
  if (treadmill->previous_config.weight != treadmill->config.weight) {
    nvm.NVM_WriteVariable8bits(NVM_U8_WEIGHT, treadmill->config.weight);
  }
}

void treadmill_update(treadmill_t *treadmill) { rt_led_update(treadmill); }
