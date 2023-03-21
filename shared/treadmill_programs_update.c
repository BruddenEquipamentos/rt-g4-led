#include "treadmill_programs_update.h"

#include "treadmill.h"
#include "treadmill_action.h"
#include "treadmill_programs.h"

typedef struct program_training_callback {
  program_id_t id;
  void (*update)(treadmill_t *treadmill);
} program_training_callback_t;

static void update_program_time(treadmill_t *treadmill) {
  if (treadmill->training.seconds >= treadmill->program.p_time.seconds) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_distance(treadmill_t *treadmill) {
  if (treadmill->training.meters >= treadmill->program.p_distance.meters) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_calories(treadmill_t *treadmill) {
  if ((treadmill->training.kcal / 1000) >= treadmill->program.p_calories.kcal) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_hiit(treadmill_t *treadmill) {
  if ((treadmill->training.seconds / 60) & 1) {
    treadmill->training.speed = treadmill->program.p_hiit.speed_t2;
  } else {
    treadmill->training.speed = treadmill->program.p_hiit.speed_t1;
  }

  if (treadmill->training.seconds >= treadmill->program.p_hiit.seconds) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_walk_30min(treadmill_t *treadmill) {
  if (treadmill->training.seconds < 30 * 60) {
    int calculated_speed = (int)treadmill->program.p_walk_30min.target_speed;
    if (treadmill->training.seconds < 5 * 60)
      calculated_speed -= 200;
    else if (treadmill->training.seconds < 10 * 60)
      calculated_speed -= 100;
    else if (treadmill->training.seconds < 20 * 60) {
    } else if (treadmill->training.seconds < 25 * 60)
      calculated_speed -= 100;
    else if (treadmill->training.seconds < 30 * 60)
      calculated_speed -= 200;

    if (calculated_speed < treadmill->config.speed.min)
      treadmill->training.speed = treadmill->config.speed.min;
    else
      treadmill->training.speed = (uint16_t)calculated_speed;

  } else {
    treadmill_finish_program(treadmill);
  }
}

static int treadmill_has_incline(treadmill_t *treadmill) {
  return treadmill->config.incline.step > 0;
}

static int treadmill_can_increment_incline(treadmill_t *treadmill) {
  return (treadmill->config.incline.step > 0) &&
         (treadmill->training.incline + treadmill->config.incline.step <=
          treadmill->config.incline.max);
}

static int treadmill_can_decrement_incline(treadmill_t *treadmill) {
  return (treadmill->config.incline.step > 0) &&
         (treadmill->training.incline - treadmill->config.incline.step >=
          treadmill->config.incline.min);
}

static void treadmill_hr_program_below_range_handler(treadmill_t *treadmill) {
  if (treadmill_has_incline(treadmill)) {
    if (treadmill_can_increment_incline(treadmill)) {
      treadmill_incline_increment(treadmill);
    } else {
      treadmill_set_dm_message(treadmill, "VEL+", 10000, 1000);
    }
  } else {
    treadmill_set_dm_message(treadmill, "VEL+", 10000, 1000);
  }
}

static void treadmill_hr_program_above_range_handler(treadmill_t *treadmill) {
  if (treadmill_has_incline(treadmill)) {
    if (treadmill_can_decrement_incline(treadmill)) {
      treadmill_incline_decrement(treadmill);
    } else {
      treadmill_set_dm_message(treadmill, "VEL-", 10000, 1000);
    }
  } else {
    treadmill_set_dm_message(treadmill, "VEL-", 10000, 1000);
  }
}

static void update_program_fat_burn(treadmill_t *treadmill) {
  struct p_fat_burn *p_fat_burn = &treadmill->program.p_fat_burn;

  switch (p_fat_burn->hr_program_state) {
    case HEART_RATE_PROGRAM_STATE_INIT: {
      p_fat_burn->tick_ms = treadmill->get_time_ms();
      p_fat_burn->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      break;
    }
    case HEART_RATE_PROGRAM_STATE_WAIT: {
      if (treadmill->get_time_ms() - p_fat_burn->tick_ms >= 10000) {
        p_fat_burn->hr_program_state = HEART_RATE_PROGRAM_STATE_ACTION;
        p_fat_burn->tick_ms = 0;
      }
      break;
    }
    case HEART_RATE_PROGRAM_STATE_ACTION: {
      if (treadmill->training.bpm < p_fat_burn->low_bpm) {
        treadmill_hr_program_below_range_handler(treadmill);
      } else if (treadmill->training.bpm > p_fat_burn->high_bpm) {
        treadmill_hr_program_above_range_handler(treadmill);
      }
      p_fat_burn->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      p_fat_burn->tick_ms = treadmill->get_time_ms();
      break;
    }
  }

  if (treadmill->training.seconds >= p_fat_burn->seconds) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_super_cardio(treadmill_t *treadmill) {
  struct p_super_cardio *p_super_cardio = &treadmill->program.p_super_cardio;

  switch (p_super_cardio->hr_program_state) {
    case HEART_RATE_PROGRAM_STATE_INIT: {
      p_super_cardio->tick_ms = treadmill->get_time_ms();
      p_super_cardio->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      break;
    }
    case HEART_RATE_PROGRAM_STATE_WAIT: {
      if (treadmill->get_time_ms() - p_super_cardio->tick_ms >= 10000) {
        p_super_cardio->hr_program_state = HEART_RATE_PROGRAM_STATE_ACTION;
        p_super_cardio->tick_ms = 0;
      }
      break;
    }
    case HEART_RATE_PROGRAM_STATE_ACTION: {
      if (treadmill->training.bpm < p_super_cardio->low_bpm) {
        treadmill_hr_program_below_range_handler(treadmill);
      } else if (treadmill->training.bpm > p_super_cardio->high_bpm) {
        treadmill_hr_program_above_range_handler(treadmill);
      }
      p_super_cardio->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      p_super_cardio->tick_ms = treadmill->get_time_ms();
      break;
    }
  }

  if (treadmill->training.seconds >= p_super_cardio->seconds) {
    treadmill_finish_program(treadmill);
  }
}

program_training_callback_t program_training_callbacks[] = {
    {.id = PROGRAM_TIME, .update = update_program_time},
    {.id = PROGRAM_DISTANCE, .update = update_program_distance},
    {.id = PROGRAM_CALORIES, .update = update_program_calories},
    {.id = PROGRAM_HIIT, .update = update_program_hiit},
    {.id = PROGRAM_WALK_30MIN, .update = update_program_walk_30min},
    {.id = PROGRAM_FAT_BURN, .update = update_program_fat_burn},
    {.id = PROGRAM_SUPER_CARDIO, .update = update_program_super_cardio},
};

void treadmill_scene_training_programs_update(treadmill_t *treadmill) {
  for (program_id_t i = 0; i < PROGRAM_TOTALS; ++i) {
    if (program_training_callbacks[i].id == treadmill->program.id) {
      program_training_callbacks[i].update(treadmill);
      return;
    }
  }
}
