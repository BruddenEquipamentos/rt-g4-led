#ifndef TREADMILL_PROGRAMS_H
#define TREADMILL_PROGRAMS_H

#include <stdint.h>

#include "treadmill_key.h"

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
  uint32_t series;
  uint32_t seconds_t1;
  uint32_t speed_t1;
  uint32_t seconds_t2;
  uint32_t speed_t2;
};

struct p_walk_30min {
  uint32_t target_speed;
};

struct p_fat_burn {
  uint8_t age;
  uint32_t seconds;
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
