#ifndef TREADMILL_H
#define TREADMILL_H

#include <stdint.h>

#include "treadmill_key.h"
#include "treadmill_programs.h"
#include "beep.h"

#define START_TRAINING_COUNTDOWN_VALUE (3)

typedef enum treadmill_scene {
  SCENE_INIT = 0,
  SCENE_IDLE,
  SCENE_PROGRAM,
  SCENE_COUNTDOWN,
  SCENE_TRAINING,
  SCENE_PAUSED,
  SCENE_REPORT,
} treadmill_scene_t;

typedef struct treadmill {
  treadmill_scene_t state;
  treadmill_scene_t previous_state;

  struct program {
    program_id_t id; // temp hack - refences

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

    uint16_t pace;

    uint8_t bpm;

    // struct {
    //   int32_t incline
    // } total;

    // struct {
    //   uint16_t speed;
    //   int16_t incline;
    //   uint8_t bpm;
    // } average;

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

  void (*beep)(beep_t beep);
  uint32_t (*get_time_ms)(void);

  uint32_t ms;

} treadmill_t;

void treadmill_setup(treadmill_t *treadmill);
void treadmill_key_press(treadmill_t *treadmill, const key_t key);
void treadmill_update(treadmill_t *treadmill);

#endif
