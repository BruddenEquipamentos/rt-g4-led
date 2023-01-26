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
  if (treadmill->training.kcal >= treadmill->program.p_calories.kcal) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_hiit(treadmill_t *treadmill) {
  if (treadmill->training.seconds >= treadmill->program.p_hiit.seconds) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_walk_30min(treadmill_t *treadmill) {
  if (treadmill->training.seconds >= 30 * 60) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_fat_burn(treadmill_t *treadmill) {
  if (treadmill->training.seconds >= treadmill->program.p_fat_burn.seconds) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_super_cardio(treadmill_t *treadmill) {
  if (treadmill->training.seconds >= treadmill->program.p_super_cardio.seconds) {
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

void treadmill_scene_training_programs_update(treadmill_t *treadmill)  {
  for (program_id_t i = 0; i < PROGRAM_TOTALS; ++i) {
    if (program_training_callbacks[i].id == treadmill->program.id) {
      program_training_callbacks[i].update(treadmill);
      return;
    }
  }
}
