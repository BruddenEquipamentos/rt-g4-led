#include "treadmill_programs.h"

#include <assert.h>

#include "treadmill.h"
#include "treadmill_action.h"

char *programs_name[] = {
    "PROGRAM_NONE", "PROGRAM_TIME",       "PROGRAM_DISTANCE", "PROGRAM_CALORIES",
    "PROGRAM_HIIT", "PROGRAM_WALK_30MIN", "PROGRAM_FAT_BURN", "PROGRAM_SUPER_CARDIO", "PROGRAM_WALK_08MIN", "PROGRAM_200K_CALORIES"
};

static void program_time_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.p_time.seconds + PROGRAM_TIME_INC_TIME_STEP <= PROGRAM_TIME_MAX_TIME)
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

static void program_distance_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.p_distance.meters + PROGRAM_DISTANCE_INC_DISTANCE_STEP <= PROGRAM_DISTANCE_MAX_DISTANCE)
        treadmill->program.p_distance.meters += PROGRAM_DISTANCE_INC_DISTANCE_STEP;
      break;
    case KEY_SPEED_DECREMENT:
      if (treadmill->program.p_distance.meters > PROGRAM_DISTANCE_MIN_DISTANCE)
        treadmill->program.p_distance.meters -= PROGRAM_DISTANCE_INC_DISTANCE_STEP;
      break;
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static void program_calories_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if ((int)treadmill->program.p_calories.kcal + (int)PROGRAM_CALORIES_INC_CALORIES_STEP <=
          PROGRAM_CALORIES_MAX_CALORIES)
        treadmill->program.p_calories.kcal += PROGRAM_CALORIES_INC_CALORIES_STEP;
      break;
    case KEY_SPEED_DECREMENT:
      if ((int)treadmill->program.p_calories.kcal - (int)PROGRAM_CALORIES_INC_CALORIES_STEP >=
          PROGRAM_CALORIES_MIN_CALORIES)
        treadmill->program.p_calories.kcal -= PROGRAM_CALORIES_INC_CALORIES_STEP;
      break;
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

/////////////////// new program
static void program_200k_calories_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
    case KEY_SPEED_DECREMENT:
      break;
    // case KEY_SPEED_INCREMENT:
    //   if ((int)treadmill->program.p_200k_calories.kcal + (int)PROGRAM_200K_CALORIES_INC_CALORIES_STEP <=
    //       PROGRAM_200K_CALORIES_MAX_CALORIES)
    //     treadmill->program.p_200k_calories.kcal += PROGRAM_200K_CALORIES_INC_CALORIES_STEP;
    //   break;
    // case KEY_SPEED_DECREMENT:
    //   if ((int)treadmill->program.p_200k_calories.kcal - (int)PROGRAM_200K_CALORIES_INC_CALORIES_STEP >=
    //       PROGRAM_200K_CALORIES_MIN_CALORIES)
    //     treadmill->program.p_200k_calories.kcal -= PROGRAM_200K_CALORIES_INC_CALORIES_STEP;
    //   break;

    case KEY_INCLINE_6: { // 30 min program key
      treadmill->program.id = PROGRAM_WALK_30MIN; // program 5
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_INCLINE_9: { // 200 kcal program key
      treadmill_exit_program(treadmill);
      break;
    }
    case KEY_INCLINE_12: { // warmup program key
      treadmill->program.id = PROGRAM_WALK_08MIN; // program 9
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static uint32_t get_speed_from_shortcut_speed_key(const kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_6:
      return 500;
      break;
    case KEY_SPEED_9:
      return 800;
      break;
    case KEY_SPEED_12:
      return 1100;
      break;
    case KEY_SPEED_15:
      return 1400;
      break;
    default:;
  }
  return 0;
}

static void program_hiit_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      switch (treadmill->program.param_index) {
        case 0:
          treadmill->program.param_index++;
          break;
        case 1:
          treadmill->program.param_index++;
          break;
        case 2:
          treadmill_start_program(treadmill);
          break;
      }
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      switch (treadmill->program.param_index) {
        case 0:
          if (treadmill->program.p_hiit.seconds + PROGRAM_HIIT_TIME_STEP <= PROGRAM_HIIT_MAX_TIME)
            treadmill->program.p_hiit.seconds += PROGRAM_HIIT_TIME_STEP;
          break;
        case 1:
          if (treadmill->program.p_hiit.speed_t1 + treadmill->config.speed.step <= treadmill->config.speed.max)
            treadmill->program.p_hiit.speed_t1 += treadmill->config.speed.step;
          break;
        case 2:
          if (treadmill->program.p_hiit.speed_t2 + treadmill->config.speed.step <= treadmill->config.speed.max)
            treadmill->program.p_hiit.speed_t2 += treadmill->config.speed.step;
          break;
      }
      break;
    case KEY_SPEED_DECREMENT:
      switch (treadmill->program.param_index) {
        case 0:
          if (treadmill->program.p_hiit.seconds - PROGRAM_HIIT_TIME_STEP >= PROGRAM_HIIT_MIN_TIME)
            treadmill->program.p_hiit.seconds -= PROGRAM_HIIT_TIME_STEP;
          break;
        case 1:
          if (treadmill->program.p_hiit.speed_t1 - treadmill->config.speed.step >= treadmill->config.speed.min)
            treadmill->program.p_hiit.speed_t1 -= treadmill->config.speed.step;
          break;
        case 2:
          if (treadmill->program.p_hiit.speed_t2 - treadmill->config.speed.step >= treadmill->config.speed.min)
            treadmill->program.p_hiit.speed_t2 -= treadmill->config.speed.step;
          break;
      }
      break;
    case KEY_SPEED_6:
    case KEY_SPEED_9:
    case KEY_SPEED_12:
    case KEY_SPEED_15:
      // switch (treadmill->program.param_index) {
      //   case 1:
      //     treadmill->program.p_hiit.speed_t1 = get_speed_from_shortcut_speed_key(key);
      //     break;
      //   case 2:
      //     treadmill->program.p_hiit.speed_t2 = get_speed_from_shortcut_speed_key(key);
      //     break;
      // }
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

void program_walk_30min_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.p_walk_30min.target_speed + treadmill->config.speed.step <= PROGRAM_WALK_30MIN_MAX_SPEED)
        treadmill->program.p_walk_30min.target_speed += treadmill->config.speed.step;
      break;
    case KEY_SPEED_DECREMENT:
      if (treadmill->program.p_walk_30min.target_speed - treadmill->config.speed.step >= PROGRAM_WALK_30MIN_MIN_SPEED)
        treadmill->program.p_walk_30min.target_speed -= treadmill->config.speed.step;
      break;
    case KEY_SPEED_6:
    case KEY_SPEED_9:
    case KEY_INCLINE_INCREMENT:
    case KEY_INCLINE_DECREMENT:
      break;
    case KEY_INCLINE_6: { // 30 min program key
      treadmill_exit_program(treadmill);
      break;
    }
    case KEY_INCLINE_9: { // 200 kcal program key
      treadmill->program.id = PROGRAM_200K_CALORIES; // program 8
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_INCLINE_12: { // warmup program key
      treadmill->program.id = PROGRAM_WALK_08MIN; // program 9
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

/////////////////////// new program
void program_walk_08min_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      treadmill_start_program(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.p_walk_08min.target_speed + treadmill->config.speed.step <= PROGRAM_WALK_08MIN_MAX_SPEED)
        treadmill->program.p_walk_08min.target_speed += treadmill->config.speed.step;
      break;
    case KEY_SPEED_DECREMENT:
      if (treadmill->program.p_walk_08min.target_speed - treadmill->config.speed.step >= PROGRAM_WALK_08MIN_MIN_SPEED)
        treadmill->program.p_walk_08min.target_speed -= treadmill->config.speed.step;
      break;
    case KEY_SPEED_6:
    case KEY_SPEED_9:
    case KEY_INCLINE_INCREMENT:
    case KEY_INCLINE_DECREMENT:
      break;
    case KEY_INCLINE_6: { // 30 min program key
      treadmill->program.id = PROGRAM_WALK_30MIN; // program 5
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_INCLINE_9: { // 200 kcal program key
      treadmill->program.id = PROGRAM_200K_CALORIES; // program 8
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_INCLINE_12: { // warmup program key
      treadmill_exit_program(treadmill);
      break;
    }
    case KEY_PROGRAM:
      treadmill_next_program(treadmill);
      break;
    default:;
  }
}

static void program_fat_burn_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      switch (treadmill->program.param_index) {
        case 0:
          treadmill->program.param_index++;
          break;
        case 1:
          treadmill->program.p_fat_burn.max_bpm = 220 - treadmill->program.p_fat_burn.age;
          treadmill->program.p_fat_burn.low_bpm = (treadmill->program.p_fat_burn.max_bpm * 60) / 100;
          treadmill->program.p_fat_burn.high_bpm = (treadmill->program.p_fat_burn.max_bpm * 75) / 100;
          treadmill_start_program(treadmill);
          break;
      }
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      switch (treadmill->program.param_index) {
        case 0:
          if (treadmill->program.p_fat_burn.seconds + PROGRAM_FAT_BURN_TIME_STEP <= PROGRAM_FAT_BURN_MAX_TIME)
            treadmill->program.p_fat_burn.seconds += PROGRAM_FAT_BURN_TIME_STEP;
          break;
        case 1:
          if (treadmill->program.p_fat_burn.age + PROGRAM_FAT_BURN_AGE_STEP <= PROGRAM_FAT_BURN_MAX_AGE)
            treadmill->program.p_fat_burn.age += PROGRAM_FAT_BURN_AGE_STEP;
          break;
      }
      break;
    case KEY_SPEED_DECREMENT:
      switch (treadmill->program.param_index) {
        case 0:
          if (treadmill->program.p_fat_burn.seconds - PROGRAM_FAT_BURN_TIME_STEP >= PROGRAM_FAT_BURN_MIN_TIME)
            treadmill->program.p_fat_burn.seconds -= PROGRAM_FAT_BURN_TIME_STEP;
          break;
        case 1:
          if (treadmill->program.p_fat_burn.age - PROGRAM_FAT_BURN_AGE_STEP >= PROGRAM_FAT_BURN_MIN_AGE)
            treadmill->program.p_fat_burn.age -= PROGRAM_FAT_BURN_AGE_STEP;
          break;
      }
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

static void program_super_cardio_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      switch (treadmill->program.param_index) {
        case 0:
          treadmill->program.param_index++;
          break;
        case 1:
          treadmill->program.p_super_cardio.max_bpm = 220 - treadmill->program.p_super_cardio.age;
          treadmill->program.p_super_cardio.low_bpm = (treadmill->program.p_super_cardio.max_bpm * 75) / 100;
          treadmill->program.p_super_cardio.high_bpm = (treadmill->program.p_super_cardio.max_bpm * 85) / 100;
          treadmill_start_program(treadmill);
          break;
      }
      break;
    case KEY_EXT_KBD_STOP:
      treadmill_exit_program(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      switch (treadmill->program.param_index) {
        case 0:
          if (treadmill->program.p_super_cardio.seconds + PROGRAM_SUPER_CARDIO_TIME_STEP <=
              PROGRAM_SUPER_CARDIO_MAX_TIME)
            treadmill->program.p_super_cardio.seconds += PROGRAM_SUPER_CARDIO_TIME_STEP;
          break;
        case 1:
          if (treadmill->program.p_super_cardio.age + PROGRAM_SUPER_CARDIO_AGE_STEP <= PROGRAM_SUPER_CARDIO_MAX_AGE)
            treadmill->program.p_super_cardio.age += PROGRAM_SUPER_CARDIO_AGE_STEP;
          break;
      }
      break;
    case KEY_SPEED_DECREMENT:
      switch (treadmill->program.param_index) {
        case 0:
          if (treadmill->program.p_super_cardio.seconds - PROGRAM_SUPER_CARDIO_TIME_STEP >=
              PROGRAM_SUPER_CARDIO_MIN_TIME)
            treadmill->program.p_super_cardio.seconds -= PROGRAM_SUPER_CARDIO_TIME_STEP;
          break;
        case 1:
          if (treadmill->program.p_super_cardio.age - PROGRAM_SUPER_CARDIO_AGE_STEP >= PROGRAM_SUPER_CARDIO_MIN_AGE)
            treadmill->program.p_super_cardio.age -= PROGRAM_SUPER_CARDIO_AGE_STEP;
          break;
      }
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
    {.id = PROGRAM_WALK_08MIN, .key_press = program_walk_08min_key_press},
    {.id = PROGRAM_200K_CALORIES, .key_press = program_200k_calories_key_press},
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
      treadmill->program.p_hiit.seconds = PROGRAM_HIIT_DEFAULT_SECONDS;
      treadmill->program.p_hiit.seconds_t1 = 0;
      treadmill->program.p_hiit.seconds_t2 = 0;
      treadmill->program.p_hiit.speed_t1 = PROGRAM_HIIT_DEFAULT_SPEED_T1;
      treadmill->program.p_hiit.speed_t2 = PROGRAM_HIIT_DEFAULT_SPEED_T2;
      break;
    }
    case PROGRAM_WALK_30MIN: {
      treadmill->program.p_walk_30min.target_speed = PROGRAM_WALK_30MIN_DEFAULT_TARGET_SPEED;
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
    case PROGRAM_WALK_08MIN: {
      treadmill->program.p_walk_08min.target_speed = PROGRAM_WALK_08MIN_DEFAULT_TARGET_SPEED;
      break;
    }
    case PROGRAM_200K_CALORIES: {
      treadmill->program.p_200k_calories.kcal = PROGRAM_200K_CALORIES_DEFAULT_CALORIES;
      break;
    }
    default:;
  }
}
