#ifndef SD_H
#define SD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum sd_flag {
  SD_FLAG_NONE = 0,
  SD_FLAG_VISIBLE = 1 << 0,
  SD_FLAG_PAD_LEADING_ZERO = 1 << 1,
} sd_flag_t;

typedef struct sd {
  sd_flag_t flag;
  char text[8];
  uint8_t dot[4];
  uint8_t digit[4];
  uint8_t visible;
  struct {
    uint32_t time_ms;
    uint32_t interval;
  } blink;
} sd_t;

void sd_set_visible(sd_t *sd, const bool visible);
void sd_set_min_sec(sd_t *sd, uint32_t seconds);
void sd_set_km_m(sd_t *sd, uint32_t meters);
void sd_set_kmh(sd_t *sd, const uint32_t speed);
void sd_set_value(sd_t *sd, const uint32_t value);
void sd_blink(sd_t *sd, const uint32_t update_time_ms,
              const uint32_t blink_interval_ms);

void sd_set_version(sd_t *sd, const uint8_t major, const uint8_t minor,
                    const uint8_t revision);
#endif
