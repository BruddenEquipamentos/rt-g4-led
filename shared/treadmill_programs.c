#include "treadmill_programs.h"

#include <assert.h>

#include "treadmill.h"
#include "treadmill_action.h"

char *programs_name[] = {
    "PROGRAM_NONE",     "PROGRAM_TIME",         "PROGRAM_DISTANCE",
    "PROGRAM_CALORIES", "PROGRAM_HIIT",         "PROGRAM_WALK_30MIN",
    "PROGRAM_FAT_BURN", "PROGRAM_SUPER_CARDIO",
};

static void program_time_key_press(treadmill_t *treadmill, const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.p_time.seconds + PROGRAM_TIME_INC_TIME_STEP <=
          PROGRAM_TIME_MAX_TIME)
        treadmill->program.p_time.seconds += PROGRAM_TIME_INC_TIME_STEP;
      break;
    case KEY_SPEED_DECREMENT:
      if (treadmill->program.p_time.seconds > PROGRAM_TIME_MIN_TIME)
        treadmill->program.p_time.seconds -= PROGRAM_TIME_INC_TIME_STEP;
      break;
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static void program_distance_key_press(treadmill_t *treadmill,
                                       const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.p_distance.meters +
              PROGRAM_DISTANCE_INC_DISTANCE_STEP <=
          PROGRAM_DISTANCE_MAX_DISTANCE)
        treadmill->program.p_distance.meters +=
            PROGRAM_DISTANCE_INC_DISTANCE_STEP;
      break;
    case KEY_SPEED_DECREMENT:
      if (treadmill->program.p_distance.meters > PROGRAM_DISTANCE_MIN_DISTANCE)
        treadmill->program.p_distance.meters -=
            PROGRAM_DISTANCE_INC_DISTANCE_STEP;
      break;
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static void program_calories_key_press(treadmill_t *treadmill,
                                       const key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.p_calories.kcal +
              PROGRAM_CALORIES_INC_CALORIES_STEP <=
          PROGRAM_CALORIES_MAX_CALORIES)
        treadmill->program.p_calories.kcal +=
            PROGRAM_CALORIES_INC_CALORIES_STEP;
    case KEY_SPEED_DECREMENT:
      if (treadmill->program.p_calories.kcal > PROGRAM_CALORIES_MIN_CALORIES)
        treadmill->program.p_calories.kcal -=
            PROGRAM_CALORIES_INC_CALORIES_STEP;
      break;
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static void program_hiit_key_press(treadmill_t *treadmill, const key_t key) {
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
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

void program_walk_30min_key_press(treadmill_t *treadmill, const key_t key) {
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
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static void program_fat_burn_key_press(treadmill_t *treadmill,
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
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static void program_super_cardio_key_press(treadmill_t *treadmill,
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
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

key_press_program_callback_t key_press_program_callback[PROGRAM_TOTALS] = {
    {.id = PROGRAM_TIME, .key_press = program_time_key_press},
    {.id = PROGRAM_DISTANCE, .key_press = program_distance_key_press},
    {.id = PROGRAM_CALORIES, .key_press = program_calories_key_press},
    {.id = PROGRAM_HIIT, .key_press = program_hiit_key_press},
    {.id = PROGRAM_WALK_30MIN, .key_press = program_walk_30min_key_press},
    {.id = PROGRAM_FAT_BURN, .key_press = program_fat_burn_key_press},
    {.id = PROGRAM_SUPER_CARDIO, .key_press = program_super_cardio_key_press},
};

void program_load_default_value(treadmill_t *treadmill) {
  assert(treadmill->scene != SCENE_TRAINING);

  switch (treadmill->program.id) {
    case PROGRAM_TIME: {
      treadmill->program.p_time.seconds = PROGRAM_TIME_DEFAULT_TIME;
      break;
    }
    case PROGRAM_DISTANCE: {
      treadmill->program.p_distance.meters = PROGRAM_DISTANCE_DEFAULT_DISTANCE;
      break;
    }
    case PROGRAM_CALORIES: {
      treadmill->program.p_calories.kcal = PROGRAM_CALORIES_DEFAULT_CALORIES;
      break;
    }
    case PROGRAM_HIIT: {
      treadmill->program.p_hiit.time = PROGRAM_HIIT_DEFAULT_TIME;
      treadmill->program.p_hiit.speed_t1 = PROGRAM_HIIT_DEFAULT_SPEED_T1;
      treadmill->program.p_hiit.speed_t2 = PROGRAM_HIIT_DEFAULT_SPEED_T2;
      break;
    }
    case PROGRAM_WALK_30MIN: {
      treadmill->program.p_walk_30min.target_speed =
          PROGRAM_WALK_30MIN_DEFAULT_TARGET_SPEED;
      break;
    }
    case PROGRAM_FAT_BURN: {
      treadmill->program.p_fat_burn.seconds = PROGRAM_FAT_BURN_DEFAULT_TIME;
      treadmill->program.p_fat_burn.age = PROGRAM_FAT_BURN_DEFAULT_AGE;
      break;
    }
    case PROGRAM_SUPER_CARDIO: {
      treadmill->program.p_super_cardio.seconds = PROGRAM_SUPER_CARDIO_DEFAULT_TIME;
      treadmill->program.p_super_cardio.age = PROGRAM_SUPER_CARDIO_DEFAULT_AGE;
      break;
    }
    default:;
  }
}
