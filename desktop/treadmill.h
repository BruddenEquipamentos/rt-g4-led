#ifndef TREADMILL_H
#define TREADMILL_H

#include <stdint.h>

#include "programs.h"

#define START_TRAINING_COUNTDOWN_VALUE (3)

typedef enum key {
  KEY_NONE,
  KEY_START,
  KEY_STOP,
  KEY_SPEED_INCREMENT,
  KEY_SPEED_DECREMENT,
  KEY_INCLINE_INCREMENT,
  KEY_INCLINE_DECREMENT,
  KEY_PROGRAM,
} key_t;

typedef enum state {
  STATE_INIT = 0,
  STATE_IDLE,
  STATE_PROGRAM,
  STATE_COUNTDOWN,
  STATE_TRAINING,
  STATE_PAUSED,
  STATE_REPORT,
} state_t;

typedef struct treadmill {
  state_t state;
  state_t previous_state;

  struct program {
    // program_id_t id; // temp hack - refences
    int id;

    struct p_time p_time;
    struct p_distance p_distance;
    struct p_calories p_calories;

  } program;

  struct training {
    uint32_t ms;
    uint32_t seconds;

    uint32_t cm;
    uint32_t meters;
    uint32_t kcal;

    uint16_t speed;
    int16_t incline;

    uint8_t bpm;
  } training;

  struct {
    struct {
      uint16_t min;
      uint16_t max;
      uint16_t step;
    } speed;
    struct {
      int16_t min;
      int16_t max;
      int16_t step;
    } incline;
  } config;
  struct {
    uint8_t value;
    uint16_t speed;
    uint32_t meters;
    int16_t incline;
    uint32_t ms;
  } countdown;

  struct debug {
    uint8_t print_states;
  } debug;

  void (*beep)(void);
  uint32_t (*get_time_ms)(void);

  uint32_t ms;

} treadmill_t;

void treadmill_setup(treadmill_t *treadmill);
void treadmill_key_press(treadmill_t *treadmill, const key_t key);
void treadmill_update(treadmill_t *treadmill);

#endif
