#include "scene.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_close_report(treadmill);
      break;
    case KEY_STOP:
      treadmill_close_report(treadmill);
      break;
    default:;
  }
}

static void update(treadmill_t *treadmill) {}

scene_t scene_report = {
    .id = SCENE_REPORT,
    .name = "REPORT",
    .keypress = keypress,
    .update = update
};
