#include "treadmill.h"

__attribute__((weak)) void beep(void) {
}

void treadmill_setup(treadmill_t *treadmill) {
  treadmill->beep = beep;
  treadmill->state = STATE_INIT;
}
