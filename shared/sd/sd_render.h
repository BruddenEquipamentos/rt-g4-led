#ifndef SD_RENDER_H
#define SD_RENDER_H

#include "sd.h"
#include "../treadmill.h"

extern sd_t sd_distance;
extern sd_t sd_time;
extern sd_t sd_speed;
extern sd_t sd_kcal;
extern sd_t sd_program;
extern sd_t sd_pace;
extern sd_t sd_incline;
extern sd_t sd_bpm;

void sd_render(treadmill_t *treadmill);
void sd_render_all_visible(bool visible);

#endif
