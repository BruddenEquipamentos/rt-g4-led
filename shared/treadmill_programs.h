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
#define PROGRAM_CALORIES_DEFAULT_CALORIES (1000)
#define PROGRAM_CALORIES_MIN_CALORIES (500)
#define PROGRAM_CALORIES_MAX_CALORIES (9900)
#define PROGRAM_CALORIES_INC_CALORIES_STEP (100)

// hiit
#define PROGRAM_HIIT_DEFAULT_TIME (10 * 60 * 1000)  // 10 min
#define PROGRAM_HIIT_DEFAULT_SPEED_T1 (300)         // 3.0 km/h
#define PROGRAM_HIIT_DEFAULT_SPEED_T2 (700)         // 7.0 km/h

// walk 30 min
#define PROGRAM_WALK_30MIN_DEFAULT_TARGET_SPEED (600)

// fat burn
#define PROGRAM_FAT_BURN_DEFAULT_TIME (10*60*1000) // 10 min
#define PROGRAM_FAT_BURN_DEFAULT_AGE (30) // 30

// super cardio
#define PROGRAM_SUPER_CARDIO_DEFAULT_TIME (10*60*1000)
#define PROGRAM_SUPER_CARDIO_DEFAULT_AGE (30)

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
  uint32_t time;
  uint32_t speed_t1;
  uint32_t speed_t2;
};

struct p_walk_30min {
  uint32_t target_speed;
};

struct p_fat_burn {
  uint32_t seconds;
  uint8_t age;
};

struct p_super_cardio {
  uint8_t age;
  uint32_t seconds;
};

typedef struct key_press_program_callback {
  program_id_t id;
  void (*key_press)(treadmill_t *treadmill, const key_t key);
} key_press_program_callback_t;

extern key_press_program_callback_t key_press_program_callback[PROGRAM_TOTALS];

void program_load_default_value(treadmill_t *treadmill);

#endif
