#include "sd.h"

#include <string.h>

#include "treadmill.h" //Lucas 15/04
#include "panel/rt_led.h"

extern treadmill_t *pTreadmill;

struct digit_map {
  char ch;
  enum sd_digit sd_digit;
};

static struct digit_map digit_map[DIGIT_TOTALS] = {
    {.ch = '0', .sd_digit = DIGIT_0},
    {.ch = '1', .sd_digit = DIGIT_1},
    {.ch = '2', .sd_digit = DIGIT_2},
    {.ch = '3', .sd_digit = DIGIT_3},
    {.ch = '4', .sd_digit = DIGIT_4},
    {.ch = '5', .sd_digit = DIGIT_5},
    {.ch = '6', .sd_digit = DIGIT_6},
    {.ch = '7', .sd_digit = DIGIT_7},
    {.ch = '8', .sd_digit = DIGIT_8},
    {.ch = '9', .sd_digit = DIGIT_9},
    {.ch = 'P', .sd_digit = DIGIT_P},
    {.ch = '_', .sd_digit = DIGIT_UNDERLINE},
    {.ch = '-', .sd_digit = DIGIT_MIDDLE},
    {.ch = 'd', .sd_digit = DIGIT_d},
    {.ch = 'f', .sd_digit = DIGIT_f},
    {.ch = 'U', .sd_digit = DIGIT_U},
    {.ch = 'E', .sd_digit = DIGIT_E},
    {.ch = 'r', .sd_digit = DIGIT_r},
    {.ch = 't', .sd_digit = DIGIT_t},
    {.ch = 'b', .sd_digit = DIGIT_b},
    {.ch = 'L', .sd_digit = DIGIT_L},
    {.ch = 'A', .sd_digit = DIGIT_A},
    {.ch = 'o', .sd_digit = DIGIT_o},
    {.ch = ' ', .sd_digit = DIGIT_EMPTY},
};

static uint32_t kmh_x100_to_mph_x100(uint32_t kmh_x100) {
  // Approximation of 0.621371 using only 32-bit integer arithmetic.
  return (kmh_x100 * 62137u) / 100000u;
}

static size_t u32_to_dec(char *out, size_t cap, uint32_t value) {
  if (cap == 0) return 0;

  char tmp[10];
  size_t len = 0;

  do {
    tmp[len++] = (char)('0' + (value % 10u));
    value /= 10u;
  } while (value != 0u && len < sizeof(tmp));

  size_t out_len = len;
  if (out_len > cap - 1u) {
    out_len = cap - 1u;
  }

  for (size_t i = 0; i < out_len; ++i) {
    out[i] = tmp[len - 1u - i];
  }
  out[out_len] = '\0';
  return out_len;
}

static void sd_set_u32_text(sd_t *sd, uint32_t value) {
  u32_to_dec(sd->text, sizeof(sd->text), value);
}

static char get_sd_digit(char ch) {
  for (int i = 0; i < DIGIT_TOTALS; ++i)
    if (digit_map[i].ch == ch) return digit_map[i].sd_digit;
  return DIGIT_EMPTY;  // if not found  return ' '
}

void sd_set_text(sd_t *sd, const char *text) {
  size_t length = strlen(text);
  if (length > 4) return;
  if (strcmp(sd->text, text) == 0) return;  // optimization
  sd->digit[3] = get_sd_digit(text[0]);
  sd->digit[2] = get_sd_digit(text[1]);
  sd->digit[1] = get_sd_digit(text[2]);
  sd->digit[0] = get_sd_digit(text[3]);
  strcpy(sd->text, text);
}

void sd_set_program_text(sd_t *sd, const int program_number) {
  char text[5];

  if (program_number > 1000) {
    text[0] = 'P';
    text[1] = '-';
    text[2] = '-';
    text[3] = '-';
    text[4] = '\0';
  } else if (program_number > 100) {
    text[0] = 'P';
    text[1] = (char)('0' + ((program_number / 100) % 10));
    text[2] = (char)('0' + ((program_number / 10) % 10));
    text[3] = (char)('0' + (program_number % 10));
    text[4] = '\0';
  } else if (program_number > 10) {
    text[0] = ' ';
    text[1] = 'P';
    text[2] = (char)('0' + ((program_number / 10) % 10));
    text[3] = (char)('0' + (program_number % 10));
    text[4] = '\0';
  } else {
    text[0] = ' ';
    text[1] = 'P';
    text[2] = (char)('0' + (program_number % 10));
    text[3] = ' ';
    text[4] = '\0';
  }

  sd_set_text(sd, text);
}

void sd_set_min_sec(sd_t *sd, const uint32_t seconds) {
  uint32_t min = seconds / 60;  // 00:00
  uint32_t sec = seconds % 60;
  sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
  sd->dot = SD_DOT_HIDDEN_AT_2;
  sd->digit[3] = ((min % 100) / 10);
  sd->digit[2] = ((min % 100) % 10);
  sd->digit[1] = sec / 10;
  sd->digit[0] = sec % 10;
  size_t len = u32_to_dec(sd->text, sizeof(sd->text), min);
  if (len + 3u >= sizeof(sd->text)) {
    len = sizeof(sd->text) - 4u;
  }
  sd->text[len++] = ':';
  sd->text[len++] = (char)('0' + ((sec / 10) % 10));
  sd->text[len++] = (char)('0' + (sec % 10));
  sd->text[len] = '\0';
}

// uint32_t teste_milhas = 0;

void sd_set_km_m(sd_t *sd, const uint32_t meters) {
  if (pTreadmill->config.unit == RT_LED_CONFIG_UNIT_MILES){
    uint32_t total_miles = (meters * 62137u) / 1000000u;
    uint32_t miles = total_miles / 100u;
    uint32_t decimal = total_miles % 100u;
    sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
    sd->digit[3] = (miles / 10);  
    sd->digit[2] = (miles % 10);         
    sd->dot = SD_DOT_AT_2;               
    sd->digit[1] = (decimal / 10);      
    sd->digit[0] = (decimal % 10);       
    size_t len = u32_to_dec(sd->text, sizeof(sd->text), miles);
    if (len + 3u >= sizeof(sd->text)) {
      len = sizeof(sd->text) - 4u;
    }
    sd->text[len++] = '.';
    sd->text[len++] = (char)('0' + ((decimal / 10) % 10));
    sd->text[len++] = (char)('0' + (decimal % 10));
    sd->text[len] = '\0';
  
  }
  else{
    uint32_t km = meters / 1000;  // 1000 -> 1.00
    uint32_t m = (meters % 1000) / 10;
    sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
    sd->digit[3] = ((km % 100) / 10);
    sd->digit[2] = ((km % 100) % 10);
    sd->dot = SD_DOT_AT_2;
    sd->digit[1] = m / 10;
    sd->digit[0] = m % 10;
    size_t len = u32_to_dec(sd->text, sizeof(sd->text), km);
    if (len + 3u >= sizeof(sd->text)) {
      len = sizeof(sd->text) - 4u;
    }
    sd->text[len++] = '.';
    sd->text[len++] = (char)('0' + ((m / 10) % 10));
    sd->text[len++] = (char)('0' + (m % 10));
    sd->text[len] = '\0';
  }
}

void sd_set_kmh(sd_t *sd, const uint32_t speed) {
  if (pTreadmill->config.unit == RT_LED_CONFIG_UNIT_MILES){
    uint32_t spd_total_miles = kmh_x100_to_mph_x100(speed);
    uint32_t spd_miles = spd_total_miles / 100u;
    uint32_t spd_decimal = spd_total_miles % 100u;
    sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
    sd->digit[3] = 0;
    sd->digit[2] = ((spd_miles % 100) / 10); 
    sd->digit[1] = ((spd_miles % 100) % 10);     
    sd->dot = SD_DOT_AT_1;               
    sd->digit[0] = (spd_decimal / 10);
    size_t len = u32_to_dec(sd->text, sizeof(sd->text), spd_miles);
    if (len + 2u >= sizeof(sd->text)) {
      len = sizeof(sd->text) - 3u;
    }
    sd->text[len++] = '.';
    sd->text[len++] = (char)('0' + ((spd_decimal / 10) % 10));
    sd->text[len] = '\0';
  }
  else{
    uint32_t spd_km = speed / 100;  // 1200 -> 12.0 km/h
    uint32_t spd_m = speed % 100;
    sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
    sd->digit[3] = 0;
    sd->digit[2] = ((spd_km % 100) / 10);
    sd->digit[1] = ((spd_km % 100) % 10);
    sd->dot = SD_DOT_AT_1;
    sd->digit[0] = spd_m / 10;
    size_t len = u32_to_dec(sd->text, sizeof(sd->text), spd_km);
    if (len + 2u >= sizeof(sd->text)) {
      len = sizeof(sd->text) - 3u;
    }
    sd->text[len++] = '.';
    sd->text[len++] = (char)('0' + ((spd_m / 10) % 10));
    sd->text[len] = '\0';
  }
}

void sd_set_kmh_report(sd_t *sd, const uint32_t speed) {
  if (pTreadmill->config.unit == RT_LED_CONFIG_UNIT_MILES){
    uint32_t spd_total_miles = kmh_x100_to_mph_x100(speed);
    uint32_t spd_miles = spd_total_miles / 100u;
    uint32_t spd_decimal = spd_total_miles % 100u;
    sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
    sd->digit[3] = ((spd_miles % 100) / 10);
    sd->digit[2] = ((spd_miles % 100) % 10);
    sd->dot = SD_DOT_AT_2;
    sd->digit[1] = spd_decimal / 10;
    sd->digit[0] = spd_decimal % 10;
    size_t len = u32_to_dec(sd->text, sizeof(sd->text), spd_miles);
    if (len + 3u >= sizeof(sd->text)) {
      len = sizeof(sd->text) - 4u;
    }
    sd->text[len++] = '.';
    sd->text[len++] = (char)('0' + ((spd_decimal / 10) % 10));
    sd->text[len++] = (char)('0' + (spd_decimal % 10));
    sd->text[len] = '\0';
  }
  else{
    uint32_t spd_km = speed / 100;  // 1234 -> 12.34 km/h
    uint32_t spd_m = speed % 100;
    sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
    sd->digit[3] = ((spd_km % 100) / 10);
    sd->digit[2] = ((spd_km % 100) % 10);
    sd->dot = SD_DOT_AT_2;
    sd->digit[1] = spd_m / 10;
    sd->digit[0] = spd_m % 10;
    size_t len = u32_to_dec(sd->text, sizeof(sd->text), spd_km);
    if (len + 3u >= sizeof(sd->text)) {
      len = sizeof(sd->text) - 4u;
    }
    sd->text[len++] = '.';
    sd->text[len++] = (char)('0' + ((spd_m / 10) % 10));
    sd->text[len++] = (char)('0' + (spd_m % 10));
    sd->text[len] = '\0';
  }
}

void sd_set_incline(sd_t *sd, const int16_t incline) {
  sd_set_value(sd, (uint32_t)incline);
  sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
  sd->dot = SD_DOT_AT_2;
}

void sd_set_version(sd_t *sd, const uint8_t major, const uint8_t minor,
                    const uint8_t revision) {
  sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
  sd->dot |= SD_DOT_AT_1 | SD_DOT_AT_2;
  sd->digit[3] = 0;
  sd->digit[2] = major;
  sd->digit[1] = minor;
  sd->digit[0] = revision;
  sd->text[0] = (char)('0' + (major % 10));
  sd->text[1] = '.';
  sd->text[2] = (char)('0' + (minor % 10));
  sd->text[3] = '.';
  sd->text[4] = (char)('0' + (revision % 10));
  sd->text[5] = '\0';
}

void sd_set_kcal(sd_t *sd, const uint32_t kcal) {
  sd_set_value(sd, kcal);
  sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
}

void sd_set_value(sd_t *sd, const uint32_t value) {
  sd->dot = SD_DOT_NO_DOT;
  sd->digit[3] = (value % 10000) / 1000;
  sd->digit[2] = (value % 1000) / 100;
  sd->digit[1] = (value % 100) / 10;
  sd->digit[0] = (value % 10);
  sd_set_u32_text(sd, value);
}

void sd_set_bpm(sd_t *sd, const uint8_t bpm) {
  sd_set_pad_leading_zero(sd, true);
  sd_set_value(sd, bpm);
}

void sd_set_bpm_detected(sd_t *sd) {
  sd->dot = SD_DOT_NO_DOT;
  sd->digit[3] = DIGIT_EMPTY;
  sd->digit[2] = DIGIT_MIDDLE;
  sd->digit[1] = DIGIT_MIDDLE;
  sd->digit[0] = DIGIT_MIDDLE;
  sd->text[0] = '-';
  sd->text[1] = '-';
  sd->text[2] = '-';
  sd->text[3] = '\0';
}

inline void sd_set_visible(sd_t *sd, const bool visible) {
  if (visible)
    sd->flag |= SD_FLAG_VISIBLE;
  else
    sd->flag &= ~SD_FLAG_VISIBLE;

  sd->visible = visible;
}

inline void sd_set_pad_leading_zero(sd_t *sd, const bool pad) {
  if (pad)
    sd->flag |= SD_FLAG_PAD_LEADING_ZERO;
  else
    sd->flag &= ~SD_FLAG_PAD_LEADING_ZERO;
}

void sd_blink(sd_t *sd, const uint32_t update_time_ms,
              const uint32_t blink_interval_ms) {
  sd->blink.interval = blink_interval_ms;
  if (update_time_ms - sd->blink.time_ms > blink_interval_ms) {
    sd->blink.time_ms = update_time_ms;
    sd_set_visible(sd, true);
  } else if (update_time_ms - sd->blink.time_ms > blink_interval_ms / 2) {
    sd_set_visible(sd, false);
  } else {
    sd_set_visible(sd, true);
  }
}
