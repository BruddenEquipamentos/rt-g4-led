#include "treadmill.h"
#include <string.h>

__attribute__((weak)) void beep(void) {
}

void treadmill_setup(treadmill_t *treadmill) {
  memset(treadmill, 0x00, sizeof(treadmill_t));
  treadmill->beep = beep;
  treadmill->state = STATE_INIT;
  treadmill->config.speed.min = 120;
  treadmill->config.speed.max = 1800;
  treadmill->config.speed.step = 10;
  treadmill->config.incline.min = 0;
  treadmill->config.incline.max = 160;
  treadmill->config.incline.step = 10;
}
