#include "rt_led.h"

#include <stddef.h>

extern scene_t scene_init;
extern scene_t scene_idle;
extern scene_t scene_emergency;
extern scene_t scene_lub;
extern scene_t scene_config;
extern scene_t scene_paused;
extern scene_t scene_training;
extern scene_t scene_countdown;
extern scene_t scene_report;
extern scene_t scene_programs;

static const scene_t *rt_led_scenes[] = {
    &scene_init,   &scene_idle,      &scene_config,
    &scene_lub,    &scene_countdown, &scene_training,
    &scene_paused, &scene_report,    &scene_programs,
    &scene_emergency,
};

static const scene_t *rt_led_current_scene(treadmill_scene_t treadmill_scene) {
  for (uint32_t i = 0; i < sizeof(rt_led_scenes) / sizeof(scene_t *); ++i) {
    if (rt_led_scenes[i]->id == treadmill_scene) {
      return rt_led_scenes[i];
    }
  }
  return NULL;
}

void rt_led_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  const scene_t *scene = rt_led_current_scene(treadmill->scene);
  if (scene == NULL) return;

  // prevent beeping on keypress if it is going to start the countdown
  if (!(treadmill->scene == SCENE_IDLE && key == KEY_START)) {
    treadmill->hw.beep(BEEP_KEYPRESS_ACK);
  }

  scene->keypress(treadmill, key);
}

void rt_led_update(treadmill_t *treadmill) {
  const scene_t *scene = rt_led_current_scene(treadmill->scene);

  if (scene == NULL) return;
  scene->update(treadmill);
}

void rt_led_render(treadmill_t *treadmill) {
  sd_render(treadmill);
  dm_render(treadmill);
}

void rt_led_load_model_params(treadmill_t *treadmill, rt_led_model_t model) {
  treadmill->config.model = model;
  switch (model) {
    case RT_LED_MODEL_150_FREQ: {
      treadmill->config.incline.min = 0;
      treadmill->config.incline.max = 0;
      treadmill->config.incline.step = 0;

      treadmill->config.speed.min = 120;
      treadmill->config.speed.max = 1800;
      treadmill->config.speed.step = 10;

      treadmill->config.weight = 70;

      // todo: check organization
      treadmill->hw.inverter.comm_type = MOTOR_COMMUNICATION_TYPE_FREQ;
      break;
    }
    case RT_LED_MODEL_150_USART: {
      treadmill->config.incline.min = 0;
      treadmill->config.incline.max = 0;
      treadmill->config.incline.step = 0;

      treadmill->config.speed.min = 120;
      treadmill->config.speed.max = 1800;
      treadmill->config.speed.step = 10;

      treadmill->config.weight = 70;

      treadmill->hw.inverter.comm_type = MOTOR_COMMUNICATION_TYPE_UART;
      break;
    }
    case RT_LED_MODEL_250_USART: {
      treadmill->config.incline.min = 0;
      treadmill->config.incline.max = 160;
      treadmill->config.incline.step = 10;

      treadmill->config.speed.min = 120;
      treadmill->config.speed.max = 1800;
      treadmill->config.speed.step = 10;

      treadmill->config.weight = 70;

      treadmill->hw.inverter.comm_type = MOTOR_COMMUNICATION_TYPE_UART;
      break;
    }
    default:;
  }
}
