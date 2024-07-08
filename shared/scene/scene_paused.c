#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_resume_training(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_finish_training(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      break;
    case KEY_SPEED_DECREMENT:
      break;
    case KEY_INCLINE_INCREMENT:
      break;
    case KEY_INCLINE_DECREMENT:
      break;
    default:;
  }
}

static void update(treadmill_t *treadmill) {}

scene_t scene_paused = {
    .id = SCENE_PAUSED,
    .name = "PAUSED",
    .keypress = keypress,
    .update = update
};
