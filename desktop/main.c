#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "input.h"
#include "render.h"
#include "../shared/treadmill.h"
#include "../shared/treadmill_update.h"
#include "../shared/dm.h"

static void sigint(int i) {
  (void)i;
  input_restore();
  exit(1);
}

static void *thread_input(void *arg) {
  int ch;
  treadmill_t *treadmill = (treadmill_t *)arg;

  input_setup();
  signal(SIGINT, sigint);

  for (;;) {
    ch = getchar();
    switch (ch) {
      case 's': {
        treadmill_key_press(treadmill, KEY_START);
        break;
      }
      case 'p': {
        treadmill_key_press(treadmill, KEY_STOP);
        break;
      }
      case 'q': {
        treadmill_key_press(treadmill, KEY_PROGRAM);
        break;
      }
      case '=': {
        treadmill_key_press(treadmill, KEY_SPEED_INCREMENT);
        break;
      }
      case '-': {
        treadmill_key_press(treadmill, KEY_SPEED_DECREMENT);
        break;
      }
      case '0': {
        treadmill_key_press(treadmill, KEY_INCLINE_INCREMENT);
        break;
      }
      case '9': {
        treadmill_key_press(treadmill, KEY_INCLINE_DECREMENT);
        break;
      }
      case 'o': {
        treadmill_key_press(treadmill, KEY_PROGRAM);
        break;
      }
      default: {
      }
    }
    usleep(1000);
  }
  return NULL;
}

static uint64_t get_time_ms() {
  struct timespec ts;
  uint64_t ms = 0;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    ms = (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
  } else {
    printf("clock_gettime failed\n");
  }
  return ms;
}

void render_dot_matrix(const uint16_t buffer[5]) {
  const int offset = 22;
  for (int row = 0; row < 5; row++) {
    for (int i = 0; i < offset; ++i) fprintf(stdout, " ");
    for (int col = 15; col >= 0; --col) {
      if ((buffer[row] >> col) & 1)
        fprintf(stdout, "▗");
      else
        fprintf(stdout, " ");
    }
    fprintf(stdout, "\n");
  }
}

static void *thread_main(void *arg) {
  treadmill_t *treadmill = (treadmill_t *)arg;
  treadmill->ms = get_time_ms();

  for (;;) {
    while (treadmill->ms < get_time_ms()) {
      treadmill->ms = treadmill->ms + 100;
      treadmill_update(treadmill);
    }
    treadmill_render(treadmill);
    usleep(1000 * 100);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  pthread_t input_thread;
  pthread_t main_thread;

  treadmill_t treadmill;
  treadmill_setup(&treadmill);
  _dm_render = render_dot_matrix;
  treadmill.get_time_ms = (uint32_t (*)(void)) get_time_ms;

  pthread_create(&input_thread, NULL, thread_input, &treadmill);
  pthread_create(&main_thread, NULL, thread_main, &treadmill);

  pthread_join(main_thread, NULL);
  pthread_join(input_thread, NULL);
  return 0;
}
