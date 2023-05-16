#include "../treadmill_motor_modbus.h"
#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  treadmill->idle_tick = treadmill->get_time_ms();
  if (key != KEY_NONE) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

static void update(treadmill_t *treadmill) {
  if (treadmill->get_time_ms() - treadmill->scene_tick >= 10500) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

const scene_t scene_emergency = {
    .id = SCENE_EMERGENCY, .name = "EMERGENCY", .keypress = keypress, .update = update};
