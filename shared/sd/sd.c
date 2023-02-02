#include "sd.h"

#include <stdio.h>
void sd_set_min_sec(sd_t *sd, const uint32_t seconds) {
  uint32_t min = seconds / 60;  // 00:00
  uint32_t sec = seconds % 60;
  sd->digit[3] = ((min % 100) / 10);
  sd->digit[2] = ((min % 100) % 10);
  sd->digit[1] = sec / 10;
  sd->digit[0] = sec % 10;
  snprintf(sd->text, 8, "%d:%02d", min, sec);
}

void sd_set_km_m(sd_t *sd, const uint32_t meters) {
  uint32_t km = meters / 1000;  // 1000 -> 1.00
  uint32_t m = (meters % 1000) / 10;
  sd->digit[3] = ((km % 100) / 10);
  sd->digit[2] = ((km % 100) % 10);
  sd->digit[1] = m / 10;
  sd->digit[0] = m % 10;
  snprintf(sd->text, 8, "%d.%02d", km, m);
}

void sd_set_kmh(sd_t *sd, const uint32_t speed) {
  uint32_t spd_km = speed / 100;  // 1200 -> 12.0 km/h
  uint32_t spd_m = speed % 100;
  sd->digit[3] = 0;
  sd->digit[2] = ((spd_km % 100) / 10);
  sd->digit[1] = ((spd_km % 100) % 10);
  sd->digit[0] = spd_m / 10;
  snprintf(sd->text, 8, "%d.%01d", spd_km, spd_m);
}

void sd_set_incline(sd_t *sd, const int value) {

}

void sd_set_version(sd_t *sd, const uint8_t major, const uint8_t minor,
                    const uint8_t revision) {
  sd->digit[3] = major;
  sd->digit[2] = minor;
  sd->digit[1] = revision;
  snprintf(sd->text, 8, "%1d.%1d.%1d", major, minor, revision);
}

void sd_set_value(sd_t *sd, const uint32_t value) {
  sd->digit[3] = (value % 10000) / 1000;
  sd->digit[2] = (value % 1000) / 100;
  sd->digit[1] = (value % 100) / 10;
  sd->digit[0] = (value % 10);
  snprintf(sd->text, 8, "%d", value);
}

inline void sd_set_visible(sd_t *sd, const bool visible) {
  if (visible)
    sd->flag |= SD_FLAG_VISIBLE;
  else
    sd->flag &= ~SD_FLAG_VISIBLE;

  sd->visible = visible;
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

