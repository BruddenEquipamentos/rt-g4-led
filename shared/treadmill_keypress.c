
#include "treadmill_keypress.h"

#include "treadmill_action.h"
#include "treadmill_programs.h"

static void treadmill_scene_init_key_press(treadmill_t *treadmill,
                                           const key_t key) {
  (void)treadmill;
  switch (key) { default:; }
}

static void treadmill_scene_lub_key_press(treadmill_t *treadmill,
                                          const key_t key) {
  if (key != KEY_NONE) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

static void treadmill_scene_idle_key_press(treadmill_t *treadmill,
                                           const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_training_countdown(treadmill);
      break;
    case KEY_PROGRAM: {
      treadmill_program(treadmill);
      break;
    }
    default:;
  }
}

static void treadmill_scene_program_key_press(treadmill_t *treadmill,
                                              const key_t key) {
  if (key == KEY_STOP) treadmill_set_scene(treadmill, SCENE_IDLE);

  // delegate
  for (int idx = 0; idx < PROGRAM_TOTALS; ++idx) {
    if (treadmill->program.id == key_press_program_callback[idx].id) {
      key_press_program_callback[idx].key_press(treadmill, key);
      return;
    }
  }
}

static void treadmill_scene_countdown_key_press(treadmill_t *treadmill,
                                                const key_t key) {
  switch (key) {
    case KEY_STOP:
      treadmill_abort_countdown(treadmill);
      break;
    default:;
  }
}

static void treadmill_scene_training_key_press(treadmill_t *treadmill,
                                               const key_t key) {
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

static void treadmill_scene_paused_key_press(treadmill_t *treadmill,
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

static void treadmill_scene_report_key_press(treadmill_t *treadmill,
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
  treadmill_scene_t state;
  void (*key_press)(treadmill_t *treadmill, const key_t key);
} key_press_callback_t;

static key_press_callback_t key_press_callback[] = {
    {.state = SCENE_INIT, .key_press = treadmill_scene_init_key_press},
    {.state = SCENE_LUB, .key_press = treadmill_scene_lub_key_press},
    {.state = SCENE_IDLE, .key_press = treadmill_scene_idle_key_press},
    {.state = SCENE_PROGRAM, .key_press = treadmill_scene_program_key_press},
    {.state = SCENE_COUNTDOWN,
     .key_press = treadmill_scene_countdown_key_press},
    {.state = SCENE_TRAINING, .key_press = treadmill_scene_training_key_press},
    {.state = SCENE_PAUSED, .key_press = treadmill_scene_paused_key_press},
    {.state = SCENE_REPORT, .key_press = treadmill_scene_report_key_press},
};

void treadmill_key_press(treadmill_t *treadmill, const key_t key) {
  treadmill->beep(BEEP_KEYPRESS_ACK);
  for (treadmill_scene_t state = SCENE_INIT;
       state < sizeof(key_press_callback) / sizeof(key_press_callback_t);
       state++) {
    if (treadmill->scene == key_press_callback[state].state) {
      key_press_callback[state].key_press(treadmill, key);
      return;
    }
  }
}
