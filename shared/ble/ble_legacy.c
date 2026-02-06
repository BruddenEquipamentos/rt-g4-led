#include "ble_legacy.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// TEMP
#include "../treadmill.h"
#include "../treadmill_action.h"
extern treadmill_t *pTreadmill;

static int set_position_after(char **ptr, const char *word) {
  size_t length = strlen(word);
  if (strncmp(*ptr, word, length) == 0) {
    *ptr += length;
    return 1;
  }
  return 0;
}

int ble_legacy_commands_handler(char *ptr) {
  int is_legacy_command = 1;
  if (set_position_after(&ptr, "set,")) {
    if (set_position_after(&ptr, "nsu=")) {
      treadmill_save_nsu(pTreadmill, ptr, strlen(ptr));
    } else if (set_position_after(&ptr, "nsp=")) {
      treadmill_save_nsp(pTreadmill, ptr, strlen(ptr));
    } else if (set_position_after(&ptr, "nsi=")) {
      treadmill_save_nsi(pTreadmill, ptr, strlen(ptr));
    } else if (set_position_after(&ptr, "time=")) {
      char timestr[16];
      // uint32_t timestamp;
      // strcpy(timestr, ptr);
      // timestamp = strtoul(ptr, NULL, 16);
      // pTreadmill->hw.update_rtc(timestamp);
    } else if (set_position_after(&ptr, "td=")) {
      uint32_t total_distance_in_meters;
      total_distance_in_meters = strtoul(ptr, NULL, 10);
      treadmill_save_total_distance(pTreadmill, total_distance_in_meters);
    } else if (set_position_after(&ptr, "thr=")) {
      uint32_t total_time_in_seconds;
      total_time_in_seconds = strtoul(ptr, NULL, 10);
      treadmill_save_total_time(pTreadmill, total_time_in_seconds);
    }
  } else if (set_position_after(&ptr, "get,")) {
    if (set_position_after(&ptr, "nsu")) {
      // todo: Return NSU
    } else if (set_position_after(&ptr, "nsp")) {
      // todo: Return NSP
    } else if (set_position_after(&ptr, "nsi")) {
      // todo: Return NSI
    }
  } else if (set_position_after(&ptr, "oad,")) {
    if (strstr(ptr, "err")) {
      // todo: abort OAD
    } else if (strstr(ptr, "timeout")) {
      // todo: abort OAD
    } else {
      uint8_t pc = (uint8_t)strtoul(ptr, NULL, 10);
      (void)pc;
      // todo: Update OAD progress information
    }
  } else if (set_position_after(&ptr, "reset")) {
    // todo: clear NSU, NSP, NSI
    // todo: reset total_distance and total_time
  } else {
    is_legacy_command = 0;
  }

  return is_legacy_command;
}
