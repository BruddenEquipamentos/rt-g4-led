#include "render.h"

#include "dot_matrix.h"

static display_t display_one = {.on_addr = 0x10, .addr = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5, 0x06, 0x07}, .max_digits = 4, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};
static display_t display_two = {.on_addr = 0x01, .addr = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5, 0x06, 0x07}, .max_digits = 4, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};
static display_t display_three = {.on_addr = 0x10, .addr = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}, .max_digits = 4, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};
static display_t display_four = {.on_addr = 0x01, .addr = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17}, .max_digits = 4, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};
static display_t display_five = {.on_addr = 0x10, .addr = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17}, .max_digits = 4, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};
static display_t display_six = {.on_addr = 0x01, .addr = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}, .max_digits = 3, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};
static display_t display_seven = {.on_addr = 0x20, .addr = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F}, .max_digits = 3, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};
static display_t display_eight = {.on_addr = 0x20, .addr = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27}, .max_digits = 3, .visible = 1, .blink = {.interval = 0, .time_ms = 0}};

display_t* display_distance = &display_one;
display_t* display_time = &display_two;
display_t* display_speed = &display_three;
display_t* display_steps = &display_four;
display_t* display_calories = &display_five;
display_t* display_spm = &display_six;
display_t* display_power = &display_seven;
display_t* display_bpm = &display_eight;

void blink_field_reset() {
  display_distance->blink.time_ms = 0;
  display_time->blink.time_ms = 0;
  display_speed->blink.time_ms = 0;
  display_steps->blink.time_ms = 0;
  display_calories->blink.time_ms = 0;
  display_spm->blink.time_ms = 0;
  display_power->blink.time_ms = 0;
  display_bpm->blink.time_ms = 0;
}

void blink_field(const uint32_t update_time_ms, display_t* display, const uint32_t blink_interval_ms) {
  display->blink.interval = blink_interval_ms;  // todo: check if it will be needed.
  if (update_time_ms - display->blink.time_ms > blink_interval_ms) {
    display->blink.time_ms = update_time_ms;
    display->visible = 1;
  } else if (update_time_ms - display->blink.time_ms > blink_interval_ms / 2) {
    display->visible = 0;
  } else {
    display->visible = 1;
  }
}

void display_helper_hide_all_displays() {
  // TODO: could make as widgets from a parent scene/window or a list of display_t
  display_distance->visible = 0;
  display_time->visible = 0;
  display_speed->visible = 0;
  display_steps->visible = 0;
  display_calories->visible = 0;
  display_spm->visible = 0;
  display_power->visible = 0;
  display_bpm->visible = 0;
}

void display_helper_show_all_displays() {
  // TODO: could make as widgets from a parent scene/window or a list of display_t
  display_distance->visible = 1;
  display_time->visible = 1;
  display_speed->visible = 1;
  display_steps->visible = 1;
  display_calories->visible = 1;
  display_spm->visible = 1;
  display_power->visible = 1;
  display_bpm->visible = 1;
}

void display_helper_update_digits(display_t* display, const uint32_t value) {
  display->digits[3] = value / 1000;
  display->digits[2] = value % 1000 / 100;
  display->digits[1] = value % 100 / 10;
  display->digits[0] = value % 10;
}

void display_helper_update_time(display_t* display, const uint32_t seconds) {
  uint32_t min = seconds / 60;
  uint32_t sec = seconds % 60;
  display->digits[3] = min / 10;
  display->digits[2] = min % 10;
  display->digits[1] = sec / 10;
  display->digits[0] = sec % 10;
}

void display_helper_update_distance(display_t* display, const uint32_t meters) {
  uint32_t km = meters / 1000;
  uint32_t m = (meters % 1000) / 10;
  display->digits[3] = km / 10;
  display->digits[2] = km % 10;
  display->digits[1] = m / 10;
  display->digits[0] = m % 10;
}

void display_helper_update_speed(display_t* display, const uint32_t kmh) {
  uint32_t spd_km = kmh / 100;
  uint32_t spd_m = kmh % 100;
  display->digits[2] = spd_km / 10;
  display->digits[1] = spd_km % 10;
  display->digits[0] = spd_m / 10;
}

static void display_helper_render_rest(display_t* display) {
  display->dot_pos = DOT_POSITION_NONE;
  display->digits[3] = DIGIT_r;
  display->digits[2] = DIGIT_E;
  display->digits[1] = DIGIT_5;
  display->digits[0] = DIGIT_t;
}

static void display_helper_render_go(display_t* display) {
  display->dot_pos = DOT_POSITION_NONE;
  display->digits[3] = DIGIT_EMPTY;
  display->digits[2] = DIGIT_6;
  display->digits[1] = DIGIT_o;
  display->digits[0] = DIGIT_EMPTY;
}

