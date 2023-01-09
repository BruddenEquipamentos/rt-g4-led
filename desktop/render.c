#include "render.h"
#include <stdio.h>

void treadmill_render(treadmill_t *treadmill) {
  printf("\033[H\033[J");
  printf("Hello, World\n");
}