#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) { default:; }
}

static void update(treadmill_t *treadmill) {
  if (treadmill->get_time_ms() - treadmill->scene_tick >= 1500) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

const scene_t scene_init = {
    .id = SCENE_INIT,
    .name = "INIT",
    .keypress = keypress,
    .update = update
};
