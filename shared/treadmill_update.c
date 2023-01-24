#include "treadmill_update.h"

#include "treadmill_action.h"

static void treadmill_scene_init_update(treadmill_t *treadmill) {
  treadmill->state = SCENE_IDLE;
}

static void treadmill_scene_idle_update(treadmill_t *treadmill) {}

static void treadmill_scene_program_update(treadmill_t *treadmill) {}

static uint16_t treadmill_compute_pace(uint16_t speed) {
  //
  return 0;
}

static void treadmill_scene_countdown_update(treadmill_t *treadmill) {
  if (treadmill->countdown.ms == 0 &&
      treadmill->countdown.value >= START_TRAINING_COUNTDOWN_VALUE) {
    treadmill->countdown.value = START_TRAINING_COUNTDOWN_VALUE;
    treadmill->countdown.ms = treadmill->ms;
    treadmill->beep(BEEP_COUNTDOWN_COUNT);
  } else if (treadmill->countdown.value > 0) {
    if ((treadmill->ms - treadmill->countdown.ms) >= 1000) {
      treadmill->countdown.ms = treadmill->ms;
      treadmill->countdown.value--;
      treadmill->beep(BEEP_COUNTDOWN_COUNT);
    }
  } else {
    treadmill->beep(BEEP_COUNTDOWN_END);
    treadmill_start_training(treadmill);
  }
}

static void treadmill_scene_training_update(treadmill_t *treadmill) {
  if (treadmill->ms - treadmill->training.ms >= 1000) {
    treadmill->training.ms = treadmill->ms;
    treadmill->training.seconds++;
    treadmill->training.cm += (treadmill->training.speed * 27778) / 100000;
    treadmill->training.meters = treadmill->training.cm / 100;
    treadmill->training.pace = treadmill_compute_pace(treadmill->training.speed);
    // compute pace // hide display_programs
  }

  // todo programs
}

static void treadmill_scene_paused_update(treadmill_t *treadmill) {}

static void treadmill_scene_report_update(treadmill_t *treadmill) {}

typedef struct update_callback {
  treadmill_scene_t state;
  void (*update)(treadmill_t *treadmill);
} update_callback_t;

static update_callback_t update_callback[] = {
    {.state = SCENE_INIT, .update = treadmill_scene_init_update},
    {.state = SCENE_IDLE, .update = treadmill_scene_idle_update},
    {.state = SCENE_PROGRAM, .update = treadmill_scene_program_update},
    {.state = SCENE_COUNTDOWN, .update = treadmill_scene_countdown_update},
    {.state = SCENE_TRAINING, .update = treadmill_scene_training_update},
    {.state = SCENE_PAUSED, .update = treadmill_scene_paused_update},
    {.state = SCENE_REPORT, .update = treadmill_scene_report_update},
};

void treadmill_update(treadmill_t *treadmill) {
  for (treadmill_scene_t state = SCENE_INIT;
       state < sizeof(update_callback) / sizeof(update_callback_t); state++) {
    if (treadmill->state == update_callback[state].state) {
      update_callback[state].update(treadmill);
      return;
    }
  }
}
