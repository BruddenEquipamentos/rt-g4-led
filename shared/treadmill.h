#ifndef TREADMILL_H
#define TREADMILL_H

#include <stdint.h>

#include "beep.h"
#include "treadmill_key.h"
#include "treadmill_programs.h"

#define START_TRAINING_COUNTDOWN_VALUE (3)
#define LUB_NOTIFICATION_SCENE_TIME (5000)
#define IDLE_SCENE_TIME (10000)

typedef enum treadmill_scene {
  SCENE_INIT = 0,
  SCENE_IDLE,
  SCENE_LUB,
  SCENE_CONFIG,
  SCENE_PROGRAM,
  SCENE_COUNTDOWN,
  SCENE_TRAINING,
  SCENE_PAUSED,
  SCENE_REPORT,
  SCENE_TOTALS,
} treadmill_scene_t;

extern char *scene_name[];
typedef struct treadmill {
  treadmill_scene_t scene;
  treadmill_scene_t previous_scene;
  uint32_t scene_tick;

  struct program {
    program_id_t id;
    struct p_time p_time;
    struct p_distance p_distance;
    struct p_calories p_calories;
    struct p_hiit p_hiit;
    struct p_walk_30min p_walk_30min;
    struct p_fat_burn p_fat_burn;
    struct p_super_cardio p_super_cardio;
    uint8_t param_index;
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

    int option;
    int model;
    int weight;
    int lub_distance;
    int modbus;
    int test;
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

void treadmill_update(treadmill_t *treadmill);
void treadmill_set_scene(treadmill_t *treadmill, treadmill_scene_t scene);

uint32_t treadmill_calculate_pace(treadmill_t *treadmill);

#endif
