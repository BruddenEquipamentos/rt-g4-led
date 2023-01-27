#ifndef RENDER_H
#define RENDER_H
#include "../shared/treadmill.h"

#define DISPLAY_FIELD_BUFFER_MAX_LENGTH 8

typedef struct display_fields {
  char output[DISPLAY_FIELD_BUFFER_MAX_LENGTH];
  struct {
    uint32_t interval;
    uint32_t time_ms;
  } blink;
  uint8_t visible;
} display_t;

extern display_t display_distance;
extern display_t display_time;
extern display_t display_speed;
extern display_t display_kcal;
extern display_t display_program;
extern display_t display_pace;
extern display_t display_incline;
extern display_t display_bpm;

void render_blink_field(display_t* display, const uint32_t update_time_ms, const uint32_t blink_interval_ms);
void render_time(display_t *display, const uint32_t seconds);
void render_distance(display_t *display, const uint32_t meters);
void render_speed(display_t *display, const uint32_t kmh);
void render_display(display_t *display, const uint32_t value);

void treadmill_render(treadmill_t *treadmill);



#endif
