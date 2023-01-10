#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "input.h"
#include "render.h"
#include "treadmill.h"
#include "update.h"

static void sigint(int i) {
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
      default: {
      }
    }
    printf("%c", ch);
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
  pthread_t input_thread;
  pthread_t main_thread;

  treadmill_t treadmill;
  treadmill_setup(&treadmill);

  pthread_create(&input_thread, NULL, thread_input, &treadmill);
  pthread_create(&main_thread, NULL, thread_main, &treadmill);

  pthread_join(main_thread, NULL);
  pthread_join(input_thread, NULL);
  return 0;
}
