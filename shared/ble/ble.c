#include "ble.h"

#include <string.h>

#include "../debug.h"

typedef enum ble_command_status {
  BLE_COMMAND_STATUS_PENDING = 0,
  BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE,
  BLE_COMMAND_STATUS_TIMEDOUT,
  BLE_COMMAND_STATUS_OK,
  BLE_COMMAND_STATUS_ERROR,
} ble_command_status_t;

typedef enum ble_process_status {
  BLE_PROCESS_STATUS_ERROR = 0,
  BLE_PROCESS_STATUS_OK,
  BLE_PROCESS_STATUS_IDLE
} ble_process_status_t;

typedef struct ble_command {
  char data[32];
  char response[16];
  ble_command_status_t status;
  uint32_t tick;
  uint32_t timeout_tick;
} ble_command_t;

#define MAX_NUM_COMMANDS 5
static ble_command_t commands[MAX_NUM_COMMANDS];
static int head = 0;
static int tail = 0;

static void ble_clear_commands(void) {
  memset(commands, 0x00, sizeof(commands));
  head = 0;
  tail = 0;
}

int ble_post_command(const char *data, const char *response,
                     const uint32_t timeout_tick) {
  ble_command_t *cmd = &commands[head];
  strncpy(cmd->data, data, 32);
  strncpy(cmd->response, response, 16);
  cmd->tick = 0;
  cmd->timeout_tick = timeout_tick;
  head = (head + 1) % MAX_NUM_COMMANDS;
  return 1;
}

int ble_process_commands(ble_t *ble) {
  if (head != tail) {
    ble_command_t *cmd = &commands[tail];
    switch (cmd->status) {
      case BLE_COMMAND_STATUS_PENDING: {
        ble->send_data((uint8_t *)cmd->data, strlen(cmd->data));
        if (strlen(cmd->response) > 0) {
          cmd->status = BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE;
          cmd->tick = ble->get_tick();
        } else {
          cmd->status = BLE_COMMAND_STATUS_OK;
        }
        break;
      }
      case BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE: {
        char response[16];
        while (1) {
          int length = ble->recv_data((uint8_t *)response, 16);
          if (length <= 0) break;
          DPRINT("Recv: %s", response);
          if (strcmp(cmd->response, "AOK") == 0) {
            if (strstr(response, "ERR")) {
              DPRINT("GOT ERROR\n");
              cmd->status = BLE_COMMAND_STATUS_ERROR;
              return BLE_PROCESS_STATUS_ERROR;
            }
          }

          if (strstr(response, cmd->response) != NULL) {
            cmd->status = BLE_COMMAND_STATUS_OK;
          }
        }
        if (ble->get_tick() - cmd->tick >= cmd->timeout_tick) {
          cmd->status = BLE_COMMAND_STATUS_TIMEDOUT;
          DPRINT("Timedout\n");
          break;
        }
        break;
      }
      case BLE_COMMAND_STATUS_OK: {
        tail = (tail + 1) % MAX_NUM_COMMANDS;
        break;
      }
      case BLE_COMMAND_STATUS_TIMEDOUT: {
        return BLE_PROCESS_STATUS_ERROR;
        break;
      }
      case BLE_COMMAND_STATUS_ERROR: {
        return BLE_PROCESS_STATUS_ERROR;
        break;
      }
    }
    return BLE_PROCESS_STATUS_OK;
  } else {
    return BLE_PROCESS_STATUS_IDLE;
  }
}

static void ble_process_init(ble_t *ble) {
  switch (ble->init_state) {
    case BLE_INIT_STATE_INIT: {
      ble_clear_commands();
      ble_post_command("\x02SID,7\n", "AOK", 1000);
      ble_post_command("\x02SDN,MovCT\n", "AOK", 1000);
      ble->init_state = BLE_INIT_STATE_PROCESS;
      break;
    }
    case BLE_INIT_STATE_PROCESS: {
      ble_process_status_t status = ble_process_commands(ble);
      if (status == BLE_PROCESS_STATUS_ERROR) {
        ble->init_state = BLE_INIT_STATE_INIT;
      } else if (status == BLE_PROCESS_STATUS_IDLE) {
        ble->init_state = BLE_INIT_STATE_READY;
      }
      break;
    }
    case BLE_INIT_STATE_READY: {
      ble->state = BLE_STATE_READY;
      break;
    }
  }
}

void ble_process(ble_t *ble) {
  switch (ble->state) {
    case BLE_STATE_INIT: {
      ble_process_init(ble);
      break;
    }
    case BLE_STATE_READY: {
      if (ble_process_commands(ble) == BLE_PROCESS_STATUS_ERROR) {
        ble->state = BLE_STATE_ERROR;
      }
      break;
    }
    case BLE_STATE_ERROR: {
      ble->init_state = BLE_INIT_STATE_INIT;
      ble->state = BLE_STATE_INIT;
      break;
    }
  }
}

void ble_reset(ble_t *ble) {
  memset(ble, 0x00, sizeof(ble_t));
  ble->state = BLE_STATE_INIT;
}
