#include "scene.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  if (key != KEY_NONE) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

static void update(treadmill_t *treadmill) {
  if (treadmill->get_time_ms() - treadmill->scene_tick >=
      LUB_NOTIFICATION_SCENE_TIME) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

scene_t scene_lub = {
    .id = SCENE_LUB,
    .name = "LUB",
    .keypress = keypress,
    .update = update,
};
