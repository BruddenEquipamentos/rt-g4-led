#ifndef TREADMILL_H
#define TREADMILL_H

#include <stdint.h>

#define START_TRAINING_COUNTDOWN_VALUE (3)

typedef enum key {
  KEY_NONE,
  KEY_START,
  KEY_STOP,
  KEY_SPEED_INCREMENT,
  KEY_SPEED_DECREMENT,
  KEY_INCLINE_INCREMENT,
  KEY_INCLINE_DECREMENT,
} key_t;

typedef enum state {
  STATE_INIT = 0,
  STATE_IDLE,
  STATE_COUNTDOWN,
  STATE_TRAINING,
  STATE_PAUSED,
  STATE_REPORT,
} state_t;

typedef struct treadmill {
  state_t state;

  struct {
    uint32_t seconds;
    uint32_t meters;
  } training;

  struct {
    uint8_t value;
    uint32_t ms;
  } countdown;

  void (*beep)(void);

  uint32_t ms;

} treadmill_t;

void treadmill_setup(treadmill_t *treadmill);
void treadmill_key_press(treadmill_t *treadmill, const key_t key);
void treadmill_update(treadmill_t *treadmill);

#endif
