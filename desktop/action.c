#include "action.h"

#include <string.h>

void treadmill_start_training_countdown(treadmill_t *treadmill) {
  treadmill->state = STATE_COUNTDOWN;
  treadmill->countdown.value = START_TRAINING_COUNTDOWN_VALUE;
  treadmill->countdown.ms = 0;
}

void treadmill_start_training(treadmill_t *treadmill) {
  treadmill->state = STATE_TRAINING;
  memset(&treadmill->training, 0x00, sizeof(struct training));
  treadmill->training.ms = treadmill->ms;
  treadmill->training.speed = treadmill->config.speed.min;
}

void treadmill_abort_countdown(treadmill_t *treadmill) {
  treadmill->state = STATE_IDLE;
}

void treadmill_pause_training(treadmill_t *treadmill) {
  treadmill->training.speed = 0;
  treadmill->state = STATE_PAUSED;
}

void treadmill_resume_training(treadmill_t *treadmill) {
  treadmill->training.speed = treadmill->config.speed.min;
  treadmill->training.ms = treadmill->ms;
  treadmill->state = STATE_TRAINING;
}

void treadmill_finish_training(treadmill_t *treadmill) {
  treadmill->state = STATE_REPORT;
}

void treadmill_close_report(treadmill_t *treadmill) {
  treadmill->state = STATE_IDLE;
  memset(&treadmill->training, 0x00, sizeof(struct training));
}

void treadmill_speed_set(treadmill_t *treadmill, const uint16_t speed) {
  if (speed >= treadmill->config.speed.min &&
      speed <= treadmill->config.speed.max) {
    treadmill->training.speed = speed;
  }
}

void treadmill_incline_set(treadmill_t *treadmill, const int16_t incline) {
  if (incline >= treadmill->config.incline.min &&
      incline <= treadmill->config.incline.max) {
    treadmill->training.incline = incline;
  }
}

void treadmill_speed_increment(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed + treadmill->config.speed.step <=
          treadmill->config.speed.max) {
    treadmill->training.speed += treadmill->config.speed.step;
  }
}

void treadmill_speed_decrement(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed - treadmill->config.speed.step >=
          treadmill->config.speed.min) {
    treadmill->training.speed -= treadmill->config.speed.step;
  }
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