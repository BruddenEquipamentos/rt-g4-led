#include "render.h"

#include <stdbool.h>
#include <stdio.h>

#include "../shared/dm/dm.h"
#include "render_program_scene.h"

display_t display_distance;
display_t display_time;
display_t display_speed;
display_t display_kcal;
display_t display_program;
display_t display_pace;
display_t display_incline;
display_t display_bpm;

void render_time(display_t *display, const uint32_t seconds) {
  uint32_t min = seconds / 60;
  uint32_t sec = seconds % 60;
  snprintf(display->output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d:%02d", min,
           sec);
}

void render_distance(display_t *display, const uint32_t meters) {
  uint32_t km = meters / 1000;
  uint32_t m = (meters % 1000) / 10;
  snprintf(display->output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d.%02d", km, m);
}

void render_speed(display_t *display, const uint32_t kmh) {
  uint32_t spd_km = kmh / 100;
  uint32_t spd_m = kmh % 100;
  snprintf(display->output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d.%02d", spd_km,
           spd_m);
}

void render_display(display_t *display, const uint32_t value) {
  snprintf(display->output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d", value);
}

static void display_print(void) {
#define V(D) D.visible ? D.output : ""

  printf("\033[H\033[J");
  printf("------------------------------------------------------------\n");
  printf("%16s%16s%16s\n", "DISTANCE", "TIME", "SPEED");
  printf("%16s%16s%16s\n\n", V(display_distance), V(display_time),
         V(display_speed));
  printf("%16s%16s%16s\n", "CALORIES", "PROGRAM", "PACE");
  printf("%16s%16s%16s\n\n", V(display_kcal), V(display_program),
         V(display_pace));
  printf("%16s%16s%16s\n", "INCLINE", "", "BPM");
  printf("%16s%16s%16s\n\n", V(display_incline), "", V(display_bpm));
  printf("------------------------------------------------------------\n");
  dm_update();
  printf("------------------------------------------------------------\n");
}

// TODO
static void render_dot_matrix(treadmill_t *treadmill) {
  if (treadmill->scene != treadmill->previous_scene) {
    switch (treadmill->scene) {
      case SCENE_PAUSED: {
        dm_reset_frame();
        break;
      }
      case SCENE_REPORT: {
        dm_reset_frame();
      }
      default:;
    }
  }

  switch (treadmill->scene) {
    case SCENE_PAUSED: {
      dm_render_text_animated("PAUSED", 100, treadmill->get_time_ms());
      break;
    }
    case SCENE_LUB: {
      dm_render_text("LUB", 15);
      break;
    }
    case SCENE_PROGRAM: {
      dm_render_text("PROG", 16);
      break;
    }
    case SCENE_REPORT: {
      dm_render_text_animated("RELATORIO", 100, treadmill->get_time_ms());
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
}

static void render_training_data(treadmill_t *treadmill) {
  uint32_t km = treadmill->training.meters / 1000;
  uint32_t m = (treadmill->training.meters % 1000) / 10;

  uint32_t min = treadmill->training.seconds / 60;
  uint32_t s = treadmill->training.seconds % 60;

  // uint32_t kcal = treadmill->training.kcal;

  uint32_t spd_km = treadmill->training.speed / 100;
  uint32_t spd_m = treadmill->training.speed % 100;

  uint32_t pace_s = treadmill_calculate_pace(treadmill);

  // todo: testar display inclinação negativa
  int16_t inc = treadmill->training.incline;

  if (display_distance.visible)
    snprintf(display_distance.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH,
             "%d.%02d", km, m);
  else
    snprintf(display_distance.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");

  if (display_time.visible)
    snprintf(display_time.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d:%02d",
             min, s);
  else
    snprintf(display_time.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");

  if (display_speed.visible)
    snprintf(display_speed.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d.%02d",
             spd_km, spd_m);
  else
    snprintf(display_speed.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");

  if (display_kcal.visible)
    snprintf(display_kcal.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d",
             treadmill->training.kcal);
  else
    snprintf(display_kcal.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");

  if (display_program.visible)
    snprintf(display_program.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "P%d",
             treadmill->program.id);
  else
    snprintf(display_program.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");

  if (display_pace.visible)
    snprintf(display_pace.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%01d.%02d",
             pace_s / 60, pace_s % 60);
  else
    snprintf(display_pace.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");

  if (display_incline.visible)
    snprintf(display_incline.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH,
             "%01d.%01d", inc / 10, inc % 10);
  else
    snprintf(display_incline.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");

  if (display_bpm.visible)
    snprintf(display_bpm.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "%d",
             treadmill->training.bpm);
  else
    snprintf(display_bpm.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "");
}

static void render_training_data_program(treadmill_t *treadmill) {
  switch (treadmill->program.id) {
    case PROGRAM_TIME: {
      uint32_t remaining =
          treadmill->program.p_time.seconds - treadmill->training.seconds;
      render_time(&display_time, remaining);
      break;
    }
    case PROGRAM_DISTANCE: {
      uint32_t remaining =
          treadmill->program.p_distance.meters - treadmill->training.meters;
      render_distance(&display_distance, remaining);
      break;
    }
    case PROGRAM_CALORIES: {
      uint32_t remaining =
          treadmill->program.p_calories.kcal - treadmill->training.kcal;
      render_display(&display_kcal, remaining);
      break;
    }
    case PROGRAM_HIIT: {
      uint32_t remaining_seconds =
          treadmill->program.p_hiit.seconds - treadmill->training.seconds;
      render_time(&display_time, remaining_seconds);
      break;
    }
    case PROGRAM_WALK_30MIN: {
      uint32_t remaining_seconds = (30 * 60) - treadmill->training.seconds;
      render_time(&display_time, remaining_seconds);
      break;
    }
    case PROGRAM_FAT_BURN: {
      uint32_t remaining_seconds =
          treadmill->program.p_fat_burn.seconds - treadmill->training.seconds;
      render_time(&display_time, remaining_seconds);
      break;
    }
    case PROGRAM_SUPER_CARDIO: {
      uint32_t remaining_seconds = treadmill->program.p_super_cardio.seconds -
                                   treadmill->training.seconds;
      render_time(&display_time, remaining_seconds);
      break;
    }
    default:
      render_training_data(treadmill);
  }
}

static void render_init(treadmill_t *treadmill) {
  display_program.visible = false;
  render_training_data(treadmill);
}

static void render_lub(treadmill_t *treadmill) {
  // dm_render_text("LU", 10);
  display_program.visible = false;
}

static void render_idle(treadmill_t *treadmill) {
  display_program.visible = false;
  render_training_data(treadmill);
}

static void render_program(treadmill_t *treadmill) {
  display_program.visible = true;
  render_training_data(treadmill);
  snprintf(display_program.output, DISPLAY_FIELD_BUFFER_MAX_LENGTH, "P%d",
           treadmill->program.id);
  render_program_scene(treadmill);
}

static void render_countdown(treadmill_t *treadmill) {
  display_program.visible = false;
  render_training_data(treadmill);
  render_training_data_program(treadmill);
}

static void render_training(treadmill_t *treadmill) {
  display_program.visible = false;
  render_training_data(treadmill);
  render_training_data_program(treadmill);
}

static void render_paused(treadmill_t *treadmill) {}

static void render_report(treadmill_t *treadmill) {
  render_training_data(treadmill);
}

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

void render_blink_field(display_t *display, const uint32_t update_time_ms,
                        const uint32_t blink_interval_ms) {
  display->blink.interval = blink_interval_ms;
  if (update_time_ms - display->blink.time_ms > blink_interval_ms) {
    display->blink.time_ms = update_time_ms;
    display->visible = 1;
  } else if (update_time_ms - display->blink.time_ms > blink_interval_ms / 2) {
    display->visible = 0;
  } else {
    display->visible = 1;
  }
}

static void render_set_all_displays_visible(bool visible) {
  display_distance.visible = visible;
  display_time.visible = visible;
  display_speed.visible = visible;
  display_kcal.visible = visible;
  display_program.visible = visible;
  display_pace.visible = visible;
  display_incline.visible = visible;
  display_bpm.visible = visible;
}

void treadmill_render(treadmill_t *treadmill) {
  render_set_all_displays_visible(true);

  for (treadmill_scene_t i = 0;
       i < sizeof(render_scene_callback) / sizeof(render_scene_callback_t);
       ++i) {
    if (treadmill->scene == render_scene_callback[i].state) {
      render_scene_callback[i].render(treadmill);
    }
  }

  render_dot_matrix(treadmill);

  display_print();

  if (treadmill->debug.print_states) {
    fprintf(stdout, "Treadmill Scene: %s\n", scene_name[treadmill->scene]);
    fprintf(stdout, "Treadmill Program Name: %s\n",
            programs_name[treadmill->program.id]);

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
