
#include "keypress.h"

#include "action.h"

static void treadmill_state_init_key_press(treadmill_t *treadmill,
                                           const key_t key) {
  switch (key) {
    case KEY_START:
      break;
    case KEY_STOP:
      break;
    case KEY_SPEED_INCREMENT:
      break;
    case KEY_SPEED_DECREMENT:
      break;
    case KEY_INCLINE_INCREMENT:
      break;
    case KEY_INCLINE_DECREMENT:
      break;
    default:;
  }
}

static void treadmill_state_idle_key_press(treadmill_t *treadmill,
                                           const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_training(treadmill);
      break;
    case KEY_STOP:
      break;
    case KEY_SPEED_INCREMENT:
      break;
    case KEY_SPEED_DECREMENT:
      break;
    case KEY_INCLINE_INCREMENT:
      break;
    case KEY_INCLINE_DECREMENT:
      break;
    default:;
  }
}

static void treadmill_state_countdown_key_press(treadmill_t *treadmill,
                                                const key_t key) {
  switch (key) {
    case KEY_START:
      break;
    case KEY_STOP:
      treadmill_abort_countdown(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      break;
    case KEY_SPEED_DECREMENT:
      break;
    case KEY_INCLINE_INCREMENT:
      break;
    case KEY_INCLINE_DECREMENT:
      break;
    default:;
  }
}

static void treadmill_state_training_key_press(treadmill_t *treadmill,
                                               const key_t key) {
  switch (key) {
    case KEY_START:
      break;
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
    default:;
  }
}

static void treadmill_state_paused_key_press(treadmill_t *treadmill,
                                             const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_resume_training(treadmill);
      break;
    case KEY_STOP:
      treadmill_finish_training(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      break;
    case KEY_SPEED_DECREMENT:
      break;
    case KEY_INCLINE_INCREMENT:
      break;
    case KEY_INCLINE_DECREMENT:
      break;
    default:;
  }
}

static void treadmill_state_report_key_press(treadmill_t *treadmill,
                                             const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_close_report(treadmill);
      break;
    case KEY_STOP:
      treadmill_close_report(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      break;
    case KEY_SPEED_DECREMENT:
      break;
    case KEY_INCLINE_INCREMENT:
      break;
    case KEY_INCLINE_DECREMENT:
      break;
    default:;
  }
}

typedef struct key_press_callback {
  state_t state;
  void (*key_press)(treadmill_t *treadmill, const key_t key);
} key_press_callback_t;

static key_press_callback_t key_press_callback[] = {
    {.state = STATE_INIT, .key_press = treadmill_state_init_key_press},
    {.state = STATE_IDLE, .key_press = treadmill_state_idle_key_press},
    {.state = STATE_COUNTDOWN,
     .key_press = treadmill_state_countdown_key_press},
    {.state = STATE_TRAINING, .key_press = treadmill_state_training_key_press},
    {.state = STATE_PAUSED, .key_press = treadmill_state_paused_key_press},
    {.state = STATE_REPORT, .key_press = treadmill_state_report_key_press},
};

void treadmill_key_press(treadmill_t *treadmill, const key_t key) {
  for (state_t state = STATE_INIT;
       state < sizeof(key_press_callback) / sizeof(key_press_callback_t);
       state++) {
    if (treadmill->state == key_press_callback[state].state) {
      key_press_callback[state].key_press(treadmill, key);
      return;
    }
  }
}
