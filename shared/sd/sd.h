#ifndef SD_H
#define SD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum sd_flag {
  SD_FLAG_NONE = 0,
  SD_FLAG_VISIBLE = 1 << 0,
  SD_FLAG_PAD_LEADING_ZERO = 1 << 1,
} sd_flag_t;

typedef enum sd_dot {
  SD_DOT_NO_DOT = 0,
  SD_DOT_AT_0 = 0x01,
  SD_DOT_AT_1 = 0x02,
  SD_DOT_AT_2 = 0x04,
  SD_DOT_AT_3 = 0x08,
  SD_DOT_HIDDEN_AT_2,
  SD_DOT_TOTALS,
} sd_dot_t;

typedef struct sd {
  sd_flag_t flag;
  sd_dot_t dot;
  char text[8];
  uint8_t digit[4];
  uint8_t visible;
  struct {
    uint32_t time_ms;
    uint32_t interval;
  } blink;
} sd_t;

enum sd_digit {
  DIGIT_0 = 0,
  DIGIT_1,
  DIGIT_2,
  DIGIT_3,
  DIGIT_4,
  DIGIT_5,
  DIGIT_6,
  DIGIT_7,
  DIGIT_8,
  DIGIT_9,
  DIGIT_P,
  DIGIT_UNDERLINE,
  DIGIT_MIDDLE,
  DIGIT_d,
  DIGIT_f,
  DIGIT_U,
  DIGIT_E,
  DIGIT_r,
  DIGIT_t,
  DIGIT_b,
  DIGIT_L,
  DIGIT_A,
  DIGIT_o,
  DIGIT_EMPTY,
  DIGIT_TOTALS
};

void sd_set_visible(sd_t *sd, const bool visible);
void sd_set_pad_leading_zero(sd_t *sd, const bool pad);

void sd_set_min_sec(sd_t *sd, uint32_t seconds);
void sd_set_km_m(sd_t *sd, uint32_t meters);
void sd_set_kmh(sd_t *sd, const uint32_t speed);
void sd_set_incline(sd_t *sd, int16_t incline);
void sd_set_kcal(sd_t *sd, const uint32_t kcal);
void sd_set_value(sd_t *sd, const uint32_t value);
void sd_blink(sd_t *sd, const uint32_t update_time_ms,
              const uint32_t blink_interval_ms);

void sd_set_version(sd_t *sd, const uint8_t major, const uint8_t minor,
                    const uint8_t revision);

void sd_set_text(sd_t *sd, const char *text);

void sd_set_program_text(sd_t *sd, const int program_number);

void sd_set_bpm(sd_t *sd, const uint8_t bpm);

#endif
