#include "action.h"

void treadmill_start_training(treadmill_t *treadmill) {
  treadmill->state = STATE_COUNTDOWN;
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