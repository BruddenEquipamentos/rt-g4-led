#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  if (key != KEY_NONE && key != KEY_LUB) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

static void update(treadmill_t *treadmill) {
  if (treadmill->key_pressed == KEY_LUB) {
    if (treadmill->get_time_ms() - treadmill->key_pressed_tick >= 3000) {
      if (treadmill->key_pressed_tick != 0xffffffff) {
        treadmill->key_pressed_tick = 0xffffffff;  // to stop blinking
        treadmill_reset_lub(treadmill);
      }
    }
  }

  if (treadmill->get_time_ms() - treadmill->scene_tick >= LUB_NOTIFICATION_SCENE_TIME) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

scene_t scene_lub = {
    .id = SCENE_LUB,
    .name = "LUB",
    .keypress = keypress,
    .update = update,
};
