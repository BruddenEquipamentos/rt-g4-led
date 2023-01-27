#include "scene.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  switch (key) {
    case KEY_STOP:
      treadmill_abort_countdown(treadmill);
      break;
    default:;
  }
}

static void update(treadmill_t *treadmill) {
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

const scene_t scene_countdown = {.id = SCENE_COUNTDOWN,
                                 .name = "IDLE",
                                 .keypress = keypress,
                                 .update = update};
