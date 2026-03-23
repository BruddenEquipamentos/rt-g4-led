#include "render_program_scene.h"

#include "../sd/sd.h"
#include "../sd/sd_render.h"
#include "rt_led.h"

#define BLINK_INTERVAL (1000)

static void format_series_4digits(char *text, uint16_t value) {
  text[0] = (char)('0' + ((value / 1000u) % 10u));
  text[1] = (char)('0' + ((value / 100u) % 10u));
  text[2] = (char)('0' + ((value / 10u) % 10u));
  text[3] = (char)('0' + (value % 10u));
  text[4] = '\0';
}

static void render_program_time(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  sd_set_visible(&sd_time, true);
  sd_set_min_sec(&sd_time, treadmill->program.p_time.seconds);
  sd_blink(&sd_time, treadmill->get_time_ms(), BLINK_INTERVAL);
}

static void render_program_distance(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  sd_set_visible(&sd_distance, true);
  sd_set_km_m(&sd_distance, treadmill->program.p_distance.meters);
  sd_blink(&sd_distance, treadmill->get_time_ms(), BLINK_INTERVAL);
}

static void render_program_calories(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  sd_set_visible(&sd_kcal, true);
  sd_set_value(&sd_kcal, treadmill->program.p_calories.kcal);
  sd_blink(&sd_kcal, treadmill->get_time_ms(), BLINK_INTERVAL);
}

/////// new program
static void render_program_200k_calories(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  sd_set_visible(&sd_kcal, true);
  sd_set_value(&sd_kcal, treadmill->program.p_200k_calories.kcal);
  sd_blink(&sd_kcal, treadmill->get_time_ms(), BLINK_INTERVAL);
}

static void render_program_hiit(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  switch (treadmill->program.param_index) {
    case 0: {  // TIME 1
      sd_set_visible(&sd_time, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_hiit.seconds_t1);
      sd_blink(&sd_time, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
    }
    case 1: {  // SPD1
      sd_set_visible(&sd_time, true);
      sd_set_visible(&sd_speed, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_hiit.seconds_t1);
      sd_set_kmh(&sd_speed, treadmill->program.p_hiit.speed_t1);
      sd_blink(&sd_speed, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
    }
    case 2: {  // TIME 2
      sd_set_visible(&sd_time, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_hiit.seconds_t2);
      sd_blink(&sd_time, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
    }
    case 3: {  // SPD2
      sd_set_visible(&sd_time, true);
      sd_set_visible(&sd_speed, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_hiit.seconds_t2);
      sd_set_kmh(&sd_speed, treadmill->program.p_hiit.speed_t2);
      sd_blink(&sd_speed, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
    }
    case 4: {  // SERIES
      char text[5];
      format_series_4digits(text, treadmill->program.p_hiit.series);
      sd_set_text(&sd_program, text);
      sd_set_visible(&sd_program, true);
      sd_blink(&sd_program, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
    }
  }
}

static void render_program_walk_30min(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  sd_set_visible(&sd_time, true);
  sd_set_visible(&sd_speed, true);
  sd_set_min_sec(&sd_time, 30 * 60);
  sd_set_kmh(&sd_speed, treadmill->program.p_walk_30min.target_speed);
  sd_blink(&sd_speed, treadmill->get_time_ms(), BLINK_INTERVAL);
}

/// new program
static void render_program_walk_08min(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  sd_set_visible(&sd_time, true);
  sd_set_visible(&sd_speed, true);
  sd_set_min_sec(&sd_time, 8 * 60);
  sd_set_kmh(&sd_speed, treadmill->program.p_walk_08min.target_speed);
  sd_blink(&sd_speed, treadmill->get_time_ms(), BLINK_INTERVAL);
}

static void render_program_fat_burn(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  switch (treadmill->program.param_index) {
    case 0:
      sd_set_visible(&sd_time, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_fat_burn.seconds);
      sd_blink(&sd_time, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
    case 1:
      sd_set_visible(&sd_time, true);
      sd_set_visible(&sd_bpm, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_fat_burn.seconds);
      sd_set_value(&sd_bpm, treadmill->program.p_fat_burn.age);
      sd_blink(&sd_bpm, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
  }
}

static void render_program_super_cardio(treadmill_t *treadmill) {
  // sd_set_visible(&sd_program, true);
  sd_set_program_text(&sd_program, treadmill->program.id);

  switch (treadmill->program.param_index) {
    case 0:
      sd_set_visible(&sd_time, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_super_cardio.seconds);
      sd_blink(&sd_time, treadmill->get_time_ms(), BLINK_INTERVAL);
      break;
    case 1:
      sd_set_visible(&sd_time, true);
      sd_set_visible(&sd_bpm, true);
      sd_set_min_sec(&sd_time, treadmill->program.p_super_cardio.seconds);
      sd_set_value(&sd_bpm, treadmill->program.p_super_cardio.age);
      sd_blink(&sd_bpm, treadmill->get_time_ms(), BLINK_INTERVAL);
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
    {.program = PROGRAM_WALK_08MIN, .render = render_program_walk_08min},
    {.program = PROGRAM_200K_CALORIES, .render = render_program_200k_calories}
};

void render_program_scene(treadmill_t *treadmill) {
  sd_render_all_visible(false);
  for (program_id_t i = 0;
       i < sizeof(render_programs) / sizeof(render_program_callback_t); ++i) {
    if (render_programs[i].program == treadmill->program.id) {
      render_programs[i].render(treadmill);
      return;
    }
  }
}
