#include "render.h"

#include <stdio.h>

#include "../shared/dm.h"

#define DISPLAY_FIELD_BUFFER_MAX_LENGTH 8

typedef struct display_fields {
  char output[DISPLAY_FIELD_BUFFER_MAX_LENGTH];
  struct {
    uint32_t interval;
    uint32_t time_ms;
  } blink;
} display_fields_t;

typedef struct display {
  display_fields_t distance;
  display_fields_t time;
  display_fields_t speed;
  display_fields_t kcal;
  display_fields_t program;
  display_fields_t pace;
  display_fields_t incline;
  display_fields_t bpm;
} display_t;

display_t *display_distance;
display_t *display_time;
display_t *display_speed;
display_t *display_program;
display_t *display_calories;
display_t *display_pace;
display_t *display_incline;
display_t *display_bpm;

static uint32_t calc_pace(const uint16_t speed) {
  return ((60000000 / speed) * 6) / 1000;
}

static void render_panel(treadmill_t *treadmill, display_t *display) {
  uint32_t km = treadmill->training.meters / 1000;
  uint32_t m = (treadmill->training.meters % 1000) / 10;

  uint32_t min = treadmill->training.seconds / 60;
  uint32_t s = treadmill->training.seconds % 60;

  uint32_t kcal = treadmill->training.kcal;

  uint32_t spd_km = treadmill->training.speed / 100;
  uint32_t spd_m = treadmill->training.speed % 100;

  uint32_t pace_s = calc_pace(treadmill->training.speed);

  // todo: testar display inclinação negativa
  int16_t incline = treadmill->training.incline;

  snprintf(display->distance.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d.%02d",
           km, m);
  snprintf(display->time.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d:%02d",
           min, s);
  snprintf(display->speed.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d.%02d",
           spd_km, spd_m);
  snprintf(display->kcal.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d",
           treadmill->training.kcal);
  snprintf(display->program.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d",
           kcal);
  snprintf(display->pace.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%01d.%02d",
           pace_s / 60, pace_s % 60);
  snprintf(display->incline.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH,
           "%01d.%01d", incline / 10, incline % 10);
  snprintf(display->bpm.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d",
           treadmill->training.bpm);

  printf("%16s%16s%16s\n", "DISTANCE", "TIME", "SPEED");
  printf("%16s%16s%16s\n\n", display->distance.output, display->time.output,
         display->speed.output);
  printf("%16s%16s%16s\n", "CALORIES", "PROGRAM", "PACE");
  printf("%16s%16s%16s\n\n", display->program.output, display->program.output,
         display->pace.output);
  printf("%16s%16s%16s\n", "INCLINE", "", "BPM");
  printf("%16s%16s%16s\n\n", display->incline.output, display->program.output, display->bpm.output);
}

static void render_dot_matrix(treadmill_t *treadmill, display_t *display) {
  (void)display;
  if (treadmill->scene != treadmill->previous_scene) {
    switch (treadmill->scene) {
      case SCENE_PAUSED: {
        dm_tick = 0;
        dm_frame = 0;
        break;
      }
      default:;
    }
  }

  if (treadmill->get_time_ms() - dm_tick > 100) {
    dm_tick = treadmill->get_time_ms();
    dm_frame++;
  }

  switch (treadmill->scene) {
    case SCENE_PAUSED: {
      int length = dm_compute_length("PAUSED");
      dm_render_text("PAUSED", dm_frame);
      if (dm_frame > length + 16) dm_frame = 0;
      break;
    }
    case SCENE_LUB: {
      dm_render_text("LUB", 15);
      break;
    }
    default: {
      if (treadmill->scene == SCENE_COUNTDOWN) {
        dm_render_number(treadmill->countdown.value);
      } else {
        dm_clear_buffer();
      }
    };
  }

  dm_render();
}

static void render_init(treadmill_t *treadmill) {}

static void render_lub(treadmill_t *treadmill) {
  // dm_render_text("LUB", 10);
}

static void render_idle(treadmill_t *treadmill) {}

static void render_program(treadmill_t *treadmill) {}

static void render_countdown(treadmill_t *treadmill) {}

static void render_training(treadmill_t *treadmill) {}

static void render_paused(treadmill_t *treadmill) {}

static void render_report(treadmill_t *treadmill) {}

typedef struct render_scene_callback {
  treadmill_scene_t state;
  void (*render)(treadmill_t *treadmill);
} render_scene_callback_t;

static render_scene_callback_t render_scene_callback[] = {
    {.state = SCENE_INIT, .render = render_init},
    {.state = SCENE_LUB, .render = render_lub},
    {.state = SCENE_IDLE, .render = render_idle},
    {.state = SCENE_PROGRAM, .render = render_program},
    {.state = SCENE_COUNTDOWN, .render = render_countdown},
    {.state = SCENE_TRAINING, .render = render_training},
    {.state = SCENE_PAUSED, .render = render_paused},
    {.state = SCENE_REPORT, .render = render_report},
};

void treadmill_render(treadmill_t *treadmill) {
  static display_t display;
  for (treadmill_scene_t i = 0;
       i < sizeof(render_scene_callback) / sizeof(render_scene_callback_t);
       ++i) {
    if (treadmill->scene == render_scene_callback[i].state) {
      render_scene_callback[i].render(treadmill);
    }
  }

  printf("\033[H\033[J");
  printf("------------------------------------------------------------\n");
  render_panel(treadmill, &display);
  printf("------------------------------------------------------------\n");
  render_dot_matrix(treadmill, &display);
  printf("------------------------------------------------------------\n");

  if (treadmill->debug.print_states) {
    fprintf(stdout, "Treadmill Scene: %s\n", scene_name[treadmill->scene]);
    fprintf(stdout, "Treadmill Program Name: %s\n", programs_name[treadmill->program.id]);

    fprintf(stdout, "Treadmill Countdown Value: %d\n",
            treadmill->countdown.value);
    fprintf(stdout, "Treadmill Training Centimeters: %d\n",
            treadmill->training.cm);
    fprintf(stdout, "Treadmill Training Meters: %d\n",
            treadmill->training.meters);
    fprintf(stdout, "Treadmill Training Seconds: %d\n",
            treadmill->training.seconds);
    fprintf(stdout, "Treadmill Training Speed: %d\n",
            treadmill->training.speed);
    fprintf(stdout, "Treadmill Training Incline: %d\n",
            treadmill->training.incline);
  }

  treadmill->previous_scene = treadmill->scene;
}
