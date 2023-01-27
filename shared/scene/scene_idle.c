#include "scene.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_training_countdown(treadmill);
      break;
    case KEY_PROGRAM: {
      treadmill_program(treadmill);
      break;
    }
    case KEY_CONFIG: {
      treadmill_config(treadmill);
      break;
    }
    default:;
  }
}

static void update(treadmill_t *treadmill) {
  if (treadmill->get_time_ms() - treadmill->scene_tick >= IDLE_SCENE_TIME) {
    treadmill_set_scene(treadmill, SCENE_LUB);
  }
}

const scene_t scene_idle = {.id = SCENE_IDLE,
                            .name = "IDLE",
                            .keypress = keypress,
                            .update = update};
