#include "action.h"

void treadmill_start_training(treadmill_t *treadmill) {
  treadmill->state = STATE_COUNTDOWN;
  treadmill->countdown.value = START_TRAINING_COUNTDOWN_VALUE;
  treadmill->countdown.ms = 0;
}

void treadmill_abort_countdown(treadmill_t *treadmill) {
  treadmill->state = STATE_IDLE;
}

void treadmill_pause_training(treadmill_t *treadmill) {
  treadmill->state = STATE_PAUSED;
}

void treadmill_resume_training(treadmill_t *treadmill) {
  treadmill->state = STATE_TRAINING;
}

void treadmill_finish_training(treadmill_t *treadmill) {
  treadmill->state = STATE_REPORT;
}

void treadmill_close_report(treadmill_t *treadmill) {
  treadmill->state = STATE_IDLE;
}