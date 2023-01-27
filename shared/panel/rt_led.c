#include "rt_led.h"

#include <stddef.h>

extern scene_t scene_init;
extern scene_t scene_idle;
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
};

static const scene_t *rt_led_current_scene(treadmill_scene_t treadmill_scene) {
  for (uint32_t i = 0; i < sizeof(rt_led_scenes) / sizeof(scene_t *); ++i) {
    if (rt_led_scenes[i]->id == treadmill_scene) {
      return rt_led_scenes[i];
    }
  }
  return NULL;
}

void rt_led_key_press(treadmill_t *treadmill, const key_t key) {
  const scene_t *scene = rt_led_current_scene(treadmill->scene);
  if (scene == NULL) return;
  treadmill->beep(BEEP_KEYPRESS_ACK);
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
