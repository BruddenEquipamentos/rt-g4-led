#include "../treadmill_motor_modbus.h"
#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  treadmill->idle_tick = treadmill->get_time_ms();
  switch (key) {
    case KEY_START:
      treadmill_start_training_countdown(treadmill);
      break;
    case KEY_PROGRAM: {
      if (treadmill->config.modbus) {
        treadmill_set_next_mb_parameter(treadmill);
      } else {
        treadmill_program(treadmill);
      }
      break;
    }
    case KEY_CONFIG: {
      treadmill_config(treadmill);
      break;
    }
    case KEY_LUB: {
      treadmill_set_scene(treadmill, SCENE_LUB);
    }
    default:;
  }
}

static void update(treadmill_t *treadmill) {
  if (treadmill->get_time_ms() - treadmill->idle_tick >= IDLE_SCENE_TIME) {
    if (!treadmill->config.modbus) treadmill_set_scene(treadmill, SCENE_LUB);
  }
}

const scene_t scene_idle = {
    .id = SCENE_IDLE, .name = "IDLE", .keypress = keypress, .update = update};
