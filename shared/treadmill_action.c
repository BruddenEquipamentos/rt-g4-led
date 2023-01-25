#include "treadmill_action.h"

#include <string.h>

#include "treadmill_programs.h"

void treadmill_start_training_countdown(treadmill_t *treadmill) {
  treadmill->countdown.value = START_TRAINING_COUNTDOWN_VALUE;
  treadmill->countdown.ms = 0;
  treadmill_set_scene(treadmill, SCENE_COUNTDOWN);
}

void treadmill_start_training(treadmill_t *treadmill) {
  memset(&treadmill->training, 0x00, sizeof(struct training));
  treadmill->training.ms = treadmill->ms;
  treadmill_set_speed(treadmill, treadmill->config.speed.min);
  treadmill_set_scene(treadmill, SCENE_TRAINING);
}

void treadmill_abort_countdown(treadmill_t *treadmill) {
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

void treadmill_pause_training(treadmill_t *treadmill) {
  treadmill_set_speed(treadmill, 0);
  treadmill_set_scene(treadmill, SCENE_PAUSED);
}

void treadmill_resume_training(treadmill_t *treadmill) {
  treadmill->training.speed = treadmill->config.speed.min;
  treadmill->training.ms = treadmill->ms;
  treadmill_set_scene(treadmill, SCENE_TRAINING);
}

void treadmill_finish_training(treadmill_t *treadmill) {
  treadmill_set_speed(treadmill, 0);
  treadmill_set_scene(treadmill, SCENE_REPORT);
}

void treadmill_close_report(treadmill_t *treadmill) {
  memset(&treadmill->training, 0x00, sizeof(struct training));
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

// void treadmill_speed_set(treadmill_t *treadmill, const uint16_t speed) {
//   if (speed >= treadmill->config.speed.min &&
//       speed <= treadmill->config.speed.max) {
//     treadmill->training.speed = speed;

//   }
// }

// void treadmill_incline_set(treadmill_t *treadmill, const int16_t incline) {
//   if (incline >= treadmill->config.incline.min &&
//       incline <= treadmill->config.incline.max) {
//     treadmill->training.incline = incline;
//   }
// }

void treadmill_speed_increment(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed + treadmill->config.speed.step <=
          treadmill->config.speed.max) {
    treadmill_set_speed(treadmill, treadmill->training.speed + treadmill->config.speed.step);
  }
}

void treadmill_speed_decrement(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed - treadmill->config.speed.step >=
          treadmill->config.speed.min) {
    treadmill_set_speed(treadmill, treadmill->training.speed - treadmill->config.speed.step);
  }
}

void treadmill_set_speed(treadmill_t *treadmill, const uint16_t speed) {
  if (speed > treadmill->config.speed.max) return;
  if (speed > 0 && speed < treadmill->config.speed.min) return;
  treadmill->training.speed = speed;
  treadmill->training.pace = treadmill_calculate_pace(treadmill);
}

void treadmill_set_incline(treadmill_t *treadmill, const int16_t incline) {
  if (incline > treadmill->config.incline.max) return;
  if (incline < treadmill->config.incline.min) return;
  treadmill->training.incline = incline;
}

void treadmill_incline_increment(treadmill_t *treadmill) {
  if (treadmill->training.incline >= treadmill->config.incline.min &&
      treadmill->training.incline + treadmill->config.incline.step <=
          treadmill->config.incline.max) {
    treadmill->training.incline += treadmill->config.incline.step;
  }
}

void treadmill_incline_decrement(treadmill_t *treadmill) {
  if (treadmill->training.incline >= treadmill->config.incline.min &&
      treadmill->training.incline - treadmill->config.incline.step >=
          treadmill->config.incline.min) {
    treadmill->training.incline -= treadmill->config.incline.step;
  }
}

void treadmill_program(treadmill_t *treadmill) {
  treadmill->program.id = 1;
  program_load_default_value(treadmill);
  treadmill_set_scene(treadmill, SCENE_PROGRAM);
}

void treadmill_next_program(treadmill_t *treadmill) {
  if (treadmill->program.id+1 >= PROGRAM_TOTALS) treadmill->program.id = 1;
  else treadmill->program.id++;
  treadmill->program.param_index = 0;
  program_load_default_value(treadmill);
}

void treadmill_exit_program(treadmill_t *treadmill) {
  memset(&treadmill->program, 0x00, sizeof(struct program));
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

void treadmill_start_program(treadmill_t *treadmill) {
  treadmill_start_training_countdown(treadmill);
}
