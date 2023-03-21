#ifndef TREADMILL_PROGRAMS_H
#define TREADMILL_PROGRAMS_H

#include <stdint.h>

#include "treadmill_key.h"

extern char *programs_name[];

typedef struct treadmill treadmill_t;

typedef enum program_id {
  PROGRAM_NONE = 0,
  PROGRAM_TIME,
  PROGRAM_DISTANCE,
  PROGRAM_CALORIES,
  PROGRAM_HIIT,
  PROGRAM_WALK_30MIN,
  PROGRAM_FAT_BURN,
  PROGRAM_SUPER_CARDIO,
  PROGRAM_TOTALS,
} program_id_t;

// 99 minutes
#define PROGRAM_TIME_DEFAULT_TIME (60 * 5)
#define PROGRAM_TIME_MIN_TIME (60 * 1)
#define PROGRAM_TIME_MAX_TIME (60 * 99)
#define PROGRAM_TIME_INC_TIME_STEP (60)

// 99000 meters
#define PROGRAM_DISTANCE_DEFAULT_DISTANCE (1000 * 5)
#define PROGRAM_DISTANCE_MIN_DISTANCE (500)
#define PROGRAM_DISTANCE_MAX_DISTANCE (1000 * 99)
#define PROGRAM_DISTANCE_INC_DISTANCE_STEP (100)

// kcal
#define PROGRAM_CALORIES_DEFAULT_CALORIES (100)
#define PROGRAM_CALORIES_MIN_CALORIES (50)
#define PROGRAM_CALORIES_MAX_CALORIES (9950)
#define PROGRAM_CALORIES_INC_CALORIES_STEP (50)

// hiit
#define PROGRAM_HIIT_DEFAULT_SECONDS (10 * 60)  // 10 min
#define PROGRAM_HIIT_DEFAULT_SPEED_T1 (300)  // 3.0 km/h
#define PROGRAM_HIIT_DEFAULT_SPEED_T2 (700)  // 7.0 km/h
#define PROGRAM_HIIT_MAX_TIME (99 * 60)
#define PROGRAM_HIIT_TIME_STEP (60)
#define PROGRAM_HIIT_MIN_TIME (3 * 60)

// walk 30 min
#define PROGRAM_WALK_30MIN_DEFAULT_TARGET_SPEED (600)

// fat burn
#define PROGRAM_FAT_BURN_DEFAULT_TIME (10 * 60)  // 10 min
#define PROGRAM_FAT_BURN_DEFAULT_AGE (30)               // 30
#define PROGRAM_FAT_BURN_TIME_STEP (60)
#define PROGRAM_FAT_BURN_MAX_TIME (99*60)
#define PROGRAM_FAT_BURN_MIN_TIME (1*60)
#define PROGRAM_FAT_BURN_AGE_STEP (1)
#define PROGRAM_FAT_BURN_MAX_AGE (100)
#define PROGRAM_FAT_BURN_MIN_AGE (14)

// super cardio
#define PROGRAM_SUPER_CARDIO_DEFAULT_TIME (10 * 60)
#define PROGRAM_SUPER_CARDIO_DEFAULT_AGE (30)
#define PROGRAM_SUPER_CARDIO_TIME_STEP (60)
#define PROGRAM_SUPER_CARDIO_MAX_TIME (99*60)
#define PROGRAM_SUPER_CARDIO_MIN_TIME (1*60)
#define PROGRAM_SUPER_CARDIO_AGE_STEP (1)
#define PROGRAM_SUPER_CARDIO_MAX_AGE (100)
#define PROGRAM_SUPER_CARDIO_MIN_AGE (14)

struct p_time {
  uint32_t seconds;
};

struct p_distance {
  uint32_t meters;
};

struct p_calories {
  uint32_t kcal;
};

struct p_hiit {

  uint32_t seconds_t1;
  uint32_t seconds_t2;

  uint32_t seconds;
  uint32_t speed_t1;
  uint32_t speed_t2;
};

struct p_walk_30min {
  uint32_t target_speed;
};

typedef enum heart_rate_program_state {
  HEART_RATE_PROGRAM_STATE_INIT = 0,
  HEART_RATE_PROGRAM_STATE_WAIT = 1,
  HEART_RATE_PROGRAM_STATE_ACTION = 2,
} heart_rate_program_state_t;

struct p_fat_burn {
  uint32_t seconds;
  uint8_t age;
  uint8_t max_bpm;
  uint8_t low_bpm;
  uint8_t high_bpm;
  uint32_t tick_ms;
  heart_rate_program_state_t hr_program_state;
};

struct p_super_cardio {
  uint32_t seconds;
  uint8_t age;
  uint8_t max_bpm;
  uint8_t low_bpm;
  uint8_t high_bpm;
  uint32_t tick_ms;
  heart_rate_program_state_t hr_program_state;
};

typedef struct key_press_program_callback {
  program_id_t id;
  void (*key_press)(treadmill_t *treadmill, const kbd_key_t key);
} key_press_program_callback_t;

extern key_press_program_callback_t key_press_program_callback[PROGRAM_TOTALS];

void program_load_default_value(treadmill_t *treadmill);

#endif
