#include "render_program_scene.h"

#include "render.h"

#define BLINK_INTERVAL (1000)

static void render_program_time(treadmill_t *treadmill) {
  render_time(&display_time, treadmill->program.p_time.seconds);
  render_blink_field(&display_time, treadmill->get_time_ms(), BLINK_INTERVAL);
}

static void render_program_distance(treadmill_t *treadmill) {
  render_distance(&display_distance, treadmill->program.p_distance.meters);
  render_blink_field(&display_distance, treadmill->get_time_ms(),
                     BLINK_INTERVAL);
}

static void render_program_calories(treadmill_t *treadmill) {
  render_display(&display_kcal, treadmill->program.p_calories.kcal);
  render_blink_field(&display_kcal, treadmill->get_time_ms(), BLINK_INTERVAL);
}

static void render_program_hiit(treadmill_t *treadmill) {
  switch (treadmill->program.param_index) {
    case 0: {  // TIME
      render_time(&display_time, treadmill->program.p_hiit.time);
      render_blink_field(&display_time, treadmill->get_time_ms(),
                         BLINK_INTERVAL);
      break;
    }
    case 1: {  // SPD1
      render_time(&display_time, treadmill->program.p_hiit.time);
      render_speed(&display_speed, treadmill->program.p_hiit.speed_t1);
      render_blink_field(&display_speed, treadmill->get_time_ms(),
                         BLINK_INTERVAL);
      break;
    }
    case 2: {  // SPD2
      render_time(&display_time, treadmill->program.p_hiit.time);
      render_speed(&display_speed, treadmill->program.p_hiit.speed_t2);
      render_blink_field(&display_speed, treadmill->get_time_ms(),
                         BLINK_INTERVAL);
    }
  }
}

static void render_program_walk_30min(treadmill_t *treadmill) {
  render_time(&display_time, 30 * 60);
  render_speed(&display_speed, treadmill->program.p_walk_30min.target_speed);
  render_blink_field(&display_speed, treadmill->get_time_ms(), BLINK_INTERVAL);
}

static void render_program_fat_burn(treadmill_t *treadmill) {
  switch (treadmill->program.param_index) {
    case 0:
      render_time(&display_time, treadmill->program.p_fat_burn.seconds);
      render_blink_field(&display_time, treadmill->get_time_ms(),
                         BLINK_INTERVAL);
      break;
    case 1:
      render_time(&display_time, treadmill->program.p_fat_burn.seconds);
      render_display(&display_bpm, treadmill->program.p_fat_burn.age);
      render_blink_field(&display_bpm, treadmill->get_time_ms(),
                         BLINK_INTERVAL);
      break;
  }
}

static void render_program_super_cardio(treadmill_t *treadmill) {
  switch (treadmill->program.param_index) {
    case 0:
      render_time(&display_time, treadmill->program.p_super_cardio.seconds);
      render_blink_field(&display_time, treadmill->get_time_ms(),
                         BLINK_INTERVAL);
      break;
    case 1:
      render_time(&display_time, treadmill->program.p_super_cardio.seconds);
      render_display(&display_bpm, treadmill->program.p_super_cardio.age);
      render_blink_field(&display_bpm, treadmill->get_time_ms(),
                         BLINK_INTERVAL);
      break;
  }
}

typedef struct render_program_callback {
  program_id_t program;
  void (*render)(treadmill_t *treadmill);
} render_program_callback_t;

render_program_callback_t render_programs[] = {
    {.program = PROGRAM_TIME, .render = render_program_time},
    {.program = PROGRAM_DISTANCE, .render = render_program_distance},
    {.program = PROGRAM_CALORIES, .render = render_program_calories},
    {.program = PROGRAM_HIIT, .render = render_program_hiit},
    {.program = PROGRAM_WALK_30MIN, .render = render_program_walk_30min},
    {.program = PROGRAM_FAT_BURN, .render = render_program_fat_burn},
    {.program = PROGRAM_SUPER_CARDIO, .render = render_program_super_cardio},
};

void render_program_scene(treadmill_t *treadmill) {
  for (program_id_t i = 0;
       i < sizeof(render_programs) / sizeof(render_program_callback_t); ++i) {
    if (render_programs[i].program == treadmill->program.id) {
      render_programs[i].render(treadmill);
      return;
    }
  }
}
