#include "render.h"
#include <stdio.h>

void treadmill_render(treadmill_t *treadmill) {
  printf("\033[H\033[J");

  if (treadmill->debug.print_states) {
    fprintf(stdout, "Treadmill State: %d\n", treadmill->state);
    fprintf(stdout, "Treadmill Countdown Value: %d\n", treadmill->countdown.value);
    fprintf(stdout, "Treadmill Training Centimeters: %d\n", treadmill->training.cm);
    fprintf(stdout, "Treadmill Training Meters: %d\n", treadmill->training.meters);
    fprintf(stdout, "Treadmill Training Seconds: %d\n", treadmill->training.seconds);
    fprintf(stdout, "Treadmill Training Speed: %d\n", treadmill->training.speed);
    fprintf(stdout, "Treadmill Training Incline: %d\n", treadmill->training.incline);
  }
}