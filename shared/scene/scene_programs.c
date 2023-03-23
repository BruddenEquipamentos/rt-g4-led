#include "../treadmill_programs.h"
#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  if (key == KEY_STOP) treadmill_set_scene(treadmill, SCENE_IDLE);

  // delegate
  for (int idx = 0; idx < PROGRAM_TOTALS; ++idx) {
    if (treadmill->program.id == key_press_program_callback[idx].id) {
      key_press_program_callback[idx].key_press(treadmill, key);
      return;
    }
  }
}

static void update(treadmill_t *treadmill) {
  // if there's no interaction for 30s, return no idle scene
  if (treadmill->get_time_ms() - treadmill->scene_tick > 30000) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

scene_t scene_programs = {.id = SCENE_PROGRAM, .name = "PROGRAMS", .keypress = keypress, .update = update};
