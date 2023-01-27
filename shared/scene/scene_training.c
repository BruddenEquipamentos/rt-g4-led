#include "scene.h"
#include "../treadmill_programs_update.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  switch (key) {
    case KEY_STOP:
      treadmill_pause_training(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      treadmill_speed_increment(treadmill);
      break;
    case KEY_SPEED_DECREMENT:
      treadmill_speed_decrement(treadmill);
      break;
    case KEY_INCLINE_INCREMENT:
      treadmill_incline_increment(treadmill);
      break;
    case KEY_INCLINE_DECREMENT:
      treadmill_incline_decrement(treadmill);
      break;
    case KEY_SPEED_5:
      treadmill_set_speed(treadmill, 500);
      break;
    case KEY_SPEED_8:
      treadmill_set_speed(treadmill, 800);
      break;
    case KEY_SPEED_11:
      treadmill_set_speed(treadmill, 1100);
      break;
    case KEY_SPEED_14:
      treadmill_set_speed(treadmill, 1400);
      break;
    case KEY_INCLINE_3:
      treadmill_set_incline(treadmill, 30);
      break;
    case KEY_INCLINE_6:
      treadmill_set_incline(treadmill, 60);
      break;
    case KEY_INCLINE_9:
      treadmill_set_incline(treadmill, 90);
      break;
    case KEY_INCLINE_12:
      treadmill_set_incline(treadmill, 120);
      break;

    default:;
  }
}

static void update(treadmill_t *treadmill) {
  if (treadmill->ms - treadmill->training.ms >= 1000) {
    treadmill->training.ms = treadmill->ms;
    treadmill->training.seconds++;
    treadmill->training.cm += (treadmill->training.speed * 27778) / 100000;
    treadmill->training.meters = treadmill->training.cm / 100;
    // compute pace // hide display_programs
  }
  // handle program specific things
  treadmill_scene_training_programs_update(treadmill);
}

scene_t scene_training = {
    .id = SCENE_TRAINING,
    .name = "IDLE",
    .keypress = keypress,
    .update = update
};
