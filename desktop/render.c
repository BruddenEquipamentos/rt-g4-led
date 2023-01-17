#include "render.h"

#include <stdio.h>

#include "display.h"
#include "dotmatrix.h"

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
  printf("%16s%16s%16s\n\n", display->incline.output, "", display->bpm.output);
}

static void render_dot_matrix(treadmill_t *treadmill, display_t *display) {
  (void)display;
  dotmatrix_offset = 22;
  static uint32_t frame_ms_start = 0;
  static int frame = 0;
  if (treadmill->state != treadmill->previous_state) {
    switch (treadmill->state) {
      case STATE_PAUSED: {
        frame_ms_start = 0;
        frame = 0;
        break;
      }
      default:;
    }
  }

  if (treadmill->get_time_ms() - frame_ms_start > 100) {
    frame_ms_start = treadmill->get_time_ms();
    frame++;
  }

  switch (treadmill->state) {
    case STATE_PAUSED: {
      int length = dm_compute_length("PAUSED");
      dotmatrix_render_text("PAUSED", frame);
      if (frame > length + 16) frame = 0;
      break;
    }
    default: {
      if (treadmill->state == STATE_COUNTDOWN) {
        dotmatrix_print_number(treadmill->countdown.value);
      } else {
        dotmatrix_clear_buffer();
      }
      dotmatrix_print_buffer();
    };
  }
}

void treadmill_render(treadmill_t *treadmill) {
  static display_t display;

  printf("\033[H\033[J");
  printf("------------------------------------------------------------\n");
  render_panel(treadmill, &display);
  printf("------------------------------------------------------------\n");
  render_dot_matrix(treadmill, &display);
  printf("------------------------------------------------------------\n");

  if (treadmill->debug.print_states) {
    fprintf(stdout, "Treadmill State: %d\n", treadmill->state);
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

  treadmill->previous_state = treadmill->state;
}
