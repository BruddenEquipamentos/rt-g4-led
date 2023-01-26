#include "scene.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_resume_training(treadmill);
      break;
    case KEY_STOP:
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

static void render(treadmill_t *treadmill) {
  // todo: specific callback to platform?
}

scene_t scene_paused = {
    .id = SCENE_PAUSED,
    .name = "PAUSED",
    .keypress = keypress,
    .update = update,
    .render = render,
};
