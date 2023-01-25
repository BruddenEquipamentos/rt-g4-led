#include "treadmill.h"
#include <string.h>
#include <stdbool.h>

char *scene_name[SCENE_TOTALS] = {
  "SCENE_INIT",
  "SCENE_IDLE",
  "SCENE_LUB",
  "SCENE_PROGRAM",
  "SCENE_COUNTDOWN",
  "SCENE_TRAINING",
  "SCENE_PAUSED",
  "SCENE_REPORT",
};


__attribute__((weak)) void beep(beep_t beep) {
  (void)beep;
}

void treadmill_setup(treadmill_t *treadmill) {
  memset(treadmill, 0x00, sizeof(treadmill_t));
  treadmill->beep = beep;
  treadmill->scene = SCENE_INIT;
  treadmill->scene_tick = 0;

  treadmill->config.speed.min = 120;
  treadmill->config.speed.max = 1800;
  treadmill->config.speed.step = 10;
  treadmill->config.incline.min = 0;
  treadmill->config.incline.max = 160;
  treadmill->config.incline.step = 10;

  treadmill->debug.print_states = true;
}

void treadmill_set_scene(treadmill_t *treadmill, treadmill_scene_t scene) {
  treadmill->scene = scene;
  treadmill->scene_tick = treadmill->get_time_ms();
}

uint32_t treadmill_calculate_pace(treadmill_t *treadmill) {
  return ((60000000 / treadmill->training.speed) * 6) / 1000;
}
