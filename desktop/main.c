#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include "../shared/ble/ble.h"
#include "../shared/dm/dm.h"
#include "../shared/nvm/nvm.h"
#include "../shared/panel/rt_led.h"
#include "../shared/treadmill.h"
#include "../shared/treadmill_ble.h"
#include "../shared/treadmill_motor.h"
#include "ble_serial.h"
#include "input.h"
#include "motor_serial.h"
#include "nvm_desktop.h"

// TEMP
treadmill_t *pTreadmill = NULL;

static void sigint(int i) {
  (void)i;
  input_restore();
  nvm.NVM_Close();
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
      case 'c': {
        treadmill_key_press(treadmill, KEY_CONFIG);
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

static uint32_t get_time_ms() {
  struct timespec ts;
  uint64_t ms = 0;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    ms = (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
  } else {
    printf("clock_gettime failed\n");
  }
  return (uint32_t)ms;
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

static char *treadmill_motor_state_description(
    uart_protocol_state_t protocol_state) {
  switch (protocol_state) {
    case UART_PROTOCOL_INIT_STATE:
      return "PROTOCOL_INIT_STATE";
    case UART_PROTOCOL_DETECTION_STATE:
      return "PROTOCOL_DETECTION_STATE";
    case UART_PROTOCOL_PROCESS_MODBUS_STATE:
      return "PROTOCOL_PROCESS_MODBUS_STATE";
    case UART_PROTOCOL_PROCESS_MOVEMENT_STATE:
      return "PROTOCOL_PROCESS_MOVEMENT_STATE";
    case UART_PROTOCOL_PROCESS_UNKNOWN:
      return "PROTOCOL_PROCESS_UNKNOWN";
  }
  return "";
}

static char *treadmill_ble_init_state_description(
    ble_init_state_t ble_init_state) {
  switch (ble_init_state) {
    case BLE_INIT_STATE_INIT:
      return "INIT_STATE_INIT";
    case BLE_INIT_STATE_PROCESS:
      return "INIT_STATE_PROCESS";
    case BLE_INIT_STATE_READY:
      return "INIT_STATE_READY";
  }
  return "";
}

static char *treadmill_ble_state_description(ble_state_t ble_state) {
  switch (ble_state) {
    case BLE_STATE_INIT:
      return "STATE_INIT";
    case BLE_STATE_READY:
      return "STATE_READY";
    case BLE_STATE_ERROR:
      return "STATE_ERROR";
  }
  return "";
}

static void display_update(treadmill_t *treadmill) {
#define SD_R(SD) SD.visible ? SD.text : ""
  fprintf(stdout, "\033[H\033[J");

  fprintf(stdout,
          "------------------------------------------------------------\n");
  fprintf(stdout, "%16s%16s%16s\n", "DISTANCE", "TIME", "SPEED");
  fprintf(stdout, "%16s%16s%16s\n\n", SD_R(sd_distance), SD_R(sd_time),
          SD_R(sd_speed));
  fprintf(stdout, "%16s%16s%16s\n", "CALORIES", "PROGRAM", "PACE");
  fprintf(stdout, "%16s%16s%16s\n\n", SD_R(sd_kcal), SD_R(sd_program),
          SD_R(sd_pace));
  fprintf(stdout, "%16s%16s%16s\n", "INCLINE", "", "BPM");
  fprintf(stdout, "%16s%16s%16s\n\n", SD_R(sd_incline), "", SD_R(sd_bpm));
  fprintf(stdout,
          "------------------------------------------------------------\n");

  dm_update();

  fprintf(stdout,
          "------------------------------------------------------------\n");

  if (treadmill->debug.print_states) {
    fprintf(stdout, "Treadmill Scene: %s\n", scene_name[treadmill->scene]);
    fprintf(stdout, "Treadmill Program Name: %s\n",
            programs_name[treadmill->program.id]);
    fprintf(stdout, "Treadmill Countdown Value: %d\n",
            treadmill->countdown.value);
    fprintf(stdout, "Treadmill Training Centimeters: %d\n",
            treadmill->training.cm);
    fprintf(stdout, "Treadmill Training Meters: %d\n",
            treadmill->training.meters);
    fprintf(stdout, "Treadmill Training Seconds: %d\n",
            treadmill->training.seconds);
    fprintf(stdout, "Treadmill Training Speed: %d\n",
            treadmill->training.speed);
    fprintf(stdout, "Treadmill Training Incline: %d\n",
            treadmill->training.incline);
    fprintf(stdout, "Treadmill Training Calories: %d\n",
            treadmill->training.kcal);
    fprintf(stdout, "\n");

    // fprintf(stdout, "Config Option: %d\n", treadmill->config.option);
    // fprintf(stdout, "Config Model: %d\n", treadmill->config.model);
    // fprintf(stdout, "Config Weight: %d\n", treadmill->config.weight);
    // fprintf(stdout, "Config Age: %d\n", treadmill->config.age);
    // fprintf(stdout, "Config Lub: %d\n", treadmill->config.lub_distance);
    // fprintf(stdout, "Config Modbus: %d\n", treadmill->config.modbus);
    // fprintf(stdout, "Config Test: %d\n", treadmill->config.test);

    fprintf(stdout, "Treadmill Speed Range - min: %d, max: %d, step: %d\n",
            treadmill->config.speed.min, treadmill->config.speed.max,
            treadmill->config.speed.step);

    fprintf(stdout, "Treadmill Incline Range - min: %d, max: %d, step: %d\n",
            treadmill->config.incline.min, treadmill->config.incline.max,
            treadmill->config.incline.step);

    fprintf(stdout, "BLE Connected: %d\n", treadmill->hw.ble.connected);
    fprintf(stdout, "BLE Connection Interval: %d\n",
            treadmill->hw.ble.connection.interval);
    fprintf(stdout, "BLE Connection Latency: %d\n",
            treadmill->hw.ble.connection.latency);
    fprintf(stdout, "BLE Connection Timeout: %d\n",
            treadmill->hw.ble.connection.timeout);
    fprintf(stdout, "BLE Connection MTU: %d\n",
            treadmill->hw.ble.connection.mtu);
    fprintf(stdout, "BLE FTMS Subscribed: %d\n",
            treadmill->hw.ble.ftms.subscribed);
    fprintf(stdout, "BLE FTMS Control Point: %d\n",
            treadmill->hw.ble.ftms.control_point_allowed);

    fprintf(stdout, "\n");

    fprintf(stdout, "\n");
    fprintf(stdout, "Total Distance: %u meters\n",
            treadmill->total.distance_in_meters);
    fprintf(stdout, "Total Time: %u seconds\n",
            treadmill->total.time_in_seconds);
    fprintf(stdout, "\n");

    fprintf(stdout, "\n");
    // fprintf(stdout, "Treadmill Init BLE State: %s\n",
    //         treadmill_ble_init_state_description(treadmill->hw.ble.init_state));
    fprintf(stdout, "Treadmill BLE State: %s\n",
            treadmill_ble_state_description(treadmill->hw.ble.state));
    fprintf(stdout, "Treadmill Motor State: %s\n",
            treadmill_motor_state_description(treadmill->hw.protocol_state));
    fprintf(stdout, "\n");
  }
}

char display = 1;

static void *thread_main(void *arg) {
  treadmill_t *treadmill = (treadmill_t *)arg;
  treadmill->ms = get_time_ms();
  for (;;) {
    while (treadmill->ms < get_time_ms()) {
      treadmill->ms = treadmill->ms + 100;
      treadmill_update(treadmill);
    }

    treadmill_ble_process(treadmill);

    treadmill_motor_process(treadmill);

    rt_led_render(treadmill);  // memory update
    if (display) display_update(treadmill);

    usleep(1000 * 100);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  if (argc > 1) {
    display = 0;
  }

  NVM_Setup();
  nvm.NVM_Init();

  pthread_t input_thread;
  pthread_t main_thread;

  treadmill_t treadmill;
  pTreadmill = &treadmill;

  ble_serial_open();
  motor_serial_open();

  treadmill_setup(&treadmill);

  _dm_update = render_dot_matrix;  // todo: refactor

  treadmill.get_time_ms = (uint32_t(*)(void))get_time_ms;

  ble_reset(&treadmill.hw.ble);
  treadmill.hw.ble.send_data = ble_serial_write;
  treadmill.hw.ble.recv_data = ble_serial_read;
  treadmill.hw.ble.get_tick = get_time_ms;

  treadmill.hw.motor.send = motor_serial_write;
  treadmill.hw.motor.recv = motor_serial_read;

  pthread_create(&input_thread, NULL, thread_input, &treadmill);
  pthread_create(&main_thread, NULL, thread_main, &treadmill);
  pthread_join(main_thread, NULL);
  pthread_join(input_thread, NULL);

  nvm.NVM_Close();

  return 0;
}
