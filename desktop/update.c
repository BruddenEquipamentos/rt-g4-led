#include "update.h"
#include "action.h"

static void treadmill_state_init_update(treadmill_t *treadmill) {
  treadmill->state = STATE_IDLE;
}

static void treadmill_state_idle_update(treadmill_t *treadmill) {}

static void treadmill_state_program_update(treadmill_t *treadmill) {

}

static void treadmill_state_countdown_update(treadmill_t *treadmill) {
  if (treadmill->countdown.ms == 0 &&
      treadmill->countdown.value >= START_TRAINING_COUNTDOWN_VALUE) {
    treadmill->countdown.value = START_TRAINING_COUNTDOWN_VALUE;
    treadmill->countdown.ms = treadmill->ms;
    treadmill->beep();
  } else if (treadmill->countdown.value > 0) {
    if ((treadmill->ms - treadmill->countdown.ms) >= 1000) {
      treadmill->countdown.ms = treadmill->ms;
      treadmill->countdown.value--;
      treadmill->beep();
    }
  } else {
    treadmill_start_training(treadmill);
  }
}

static void treadmill_state_training_update(treadmill_t *treadmill) {
  if (treadmill->ms - treadmill->training.ms >= 1000) {
    treadmill->training.ms = treadmill->ms;
    treadmill->training.seconds++;
    treadmill->training.cm += (treadmill->training.speed * 27778) / 100000;
    treadmill->training.meters = treadmill->training.cm/100;
  }

  // todo programs
}

static void treadmill_state_paused_update(treadmill_t *treadmill) {}

static void treadmill_state_report_update(treadmill_t *treadmill) {}

typedef struct update_callback {
  state_t state;
  void (*update)(treadmill_t *treadmill);
} update_callback_t;

static update_callback_t update_callback[] = {
    {.state = STATE_INIT, .update = treadmill_state_init_update},
    {.state = STATE_IDLE, .update = treadmill_state_idle_update},
    {.state = STATE_PROGRAM, .update = treadmill_state_program_update},
    {.state = STATE_COUNTDOWN, .update = treadmill_state_countdown_update},
    {.state = STATE_TRAINING, .update = treadmill_state_training_update},
    {.state = STATE_PAUSED, .update = treadmill_state_paused_update},
    {.state = STATE_REPORT, .update = treadmill_state_report_update},
};

void treadmill_update(treadmill_t *treadmill) {
  for (state_t state = STATE_INIT;
       state < sizeof(update_callback) / sizeof(update_callback_t); state++) {
    if (treadmill->state == update_callback[state].state) {
      update_callback[state].update(treadmill);
      return;
    }
  }
}