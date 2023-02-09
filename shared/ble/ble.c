#include "ble.h"

#include <string.h>

#include "../debug.h"
#include "./ble_protocol/ble_protocol_helper.h"

typedef enum ble_command_status {
  BLE_COMMAND_STATUS_PENDING = 0,
  BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE,
  BLE_COMMAND_STATUS_TIMEDOUT,
  BLE_COMMAND_STATUS_OK,
  BLE_COMMAND_STATUS_ERROR,
} ble_command_status_t;

typedef struct ble_command {
  char data[32];
  char response[16];
  ble_command_status_t status;
  uint32_t tick;
  uint32_t timeout_tick;
} ble_command_t;

#define MAX_NUM_COMMANDS 8
static ble_command_t commands[MAX_NUM_COMMANDS];
static int head = 0;
static int tail = 0;

void ble_clear_commands(void) {
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
  cmd->status = BLE_COMMAND_STATUS_PENDING;
  head = (head + 1) % MAX_NUM_COMMANDS;
  return 1;
}

static void temp_debug_print(const char *response, const int length) {
  DPRINT("Recv: %s", response);
  for (int i = 0; i < length; ++i) {
    DPRINT("%02X ", response[i]);
  }
  DPRINT("\n");
}

static int is_expected_response(ble_command_t *cmd, const char *response,
                                const int length) {
  return strstr(response, cmd->response) != NULL;
}

static int is_error_response(ble_command_t *cmd, const char *response,
                             const int length) {
  if (strcmp(cmd->response, "AOK") == 0) {
    if (strstr(response, "ERR")) {
      DPRINT("GOT ERROR\n");
      return 1;
    }
  }
  return 0;
}

static int is_timedout(ble_t *ble, ble_command_t *cmd) {
  if (ble->get_tick() - cmd->tick >= cmd->timeout_tick) {
    DPRINT("Timedout\n");
    return 1;
  }
  return 0;
}

static void ble_protocol_process(uint8_t *ptr_data, int length) {
  if (!ble_protocol_valid_message(ptr_data, length)) {
    return;
  }

  char *ptr = (char *)&ptr_data[1];
  uint8_t opcode = next_u8(&ptr);

  switch (opcode) {
    case BP_PROGRAMMING_START: {
      /* treadmill_programming_start(treadmill); */
      break;
    }
    case BP_PROGRAMMING_INTERVAL: {
      /* uint16_t speed = next_u16_le(&ptr); */
      /* int16_t incline = next_i16_le(&ptr); */
      /* uint16_t t = next_u16_le(&ptr); */
      /* treadmill_programming_interval(treadmill, speed, incline, t); */
      break;
    }
    case BP_PROGRAMMING_FINISH: {
      /* treadmill_programming_finish(treadmill); */
      break;
    }
    case BP_PROGRAMMING_CLEAR: {
      /* treadmill_programming_clear(treadmill); */
      break;
    }
    case BP_SUBSCRIBE: {
      // movble_treadmill_subscribed(ptr);
      break;
    }
    case BP_UNSUBSCRIBE: {
      // movble_treadmill_unsubscribed(ptr);
      break;
    }
    case BP_CONTROL_POINT: {
      // movble_treadmill_ftms_control_point(ptr);
      break;
    }
    case BP_LINKINFO: {
      // tble.connection.interval = next_u16_le(&ptr);
      // tble.connection.latency = next_u16_le(&ptr);
      // tble.connection.timeout = next_u16_le(&ptr);
      // tble.connection.mtu = next_u16_le(&ptr);
      break;
    }
    case BP_BLE_HEARTRATE_CONNECTING: {
      // tble.heartrate.connecting = 1;
      // if (_movble_treadmill_heartrate_connecting != NULL)
      //   _movble_treadmill_heartrate_connecting();
      break;
    }
    case BP_BLE_HEARTRATE: {
      // uint8_t bpm;
      // tble.heartrate.connecting = 0;
      // bpm = next_u8(&ptr);
      // if (_movble_treadmill_heartrate_data != NULL)
      //   _movble_treadmill_heartrate_data(bpm);
      break;
    }
    case BP_BLE_HEARTRATE_DISCONNECT: {
      // tble.heartrate.connecting = 0;
      // if (_movble_treadmill_heartrate_disconnect != NULL)
      //   _movble_treadmill_heartrate_disconnect();
      break;
    }
    default:;
  }
}

int ble_process_received(ble_t *ble, char *data, int length) {
  static uint8_t mBuffer[256] = {0};
  static int mBufferIndex = 0;
  for (int i = 0; i < length; ++i) {
    uint8_t byte = (uint8_t)data[i];
    switch (byte) {
      case 0x01:
      case 0x02:
        mBufferIndex = 0;
        break;

      case 0x03:
      case 0x04:
      case 0x0A: {
        mBuffer[mBufferIndex] = 0;
        handle_etx_nl((char *)mBuffer);
        mBufferIndex = 0;
        break;
      }
      case 0xF0:
      case 0xF1:
        mBufferIndex = 0;
        mBuffer[mBufferIndex++] = byte;
        break;

      case 0xF5:
      case 0xF6:
        mBuffer[mBufferIndex] = '\0';
        ble_protocol_process(mBuffer, mBufferIndex);
        break;

      // case 0x06:
      //   mBuffer[0] = 0x06;
      //   xSemaphoreGiveFromISR(xSemaphoreRx, 0);
      //   break;

      case 0x0F:
        movble_treadmill_connected();
        break;
      case 0x19:
      case 0x0E:
        movble_treadmill_disconnected();
        break;
      default: {
        mBuffer[mBufferIndex++] = byte;
      }
    }
  }
  // If there's a command processing
  if (tail != head) {
    ble_command_t *cmd = &commands[tail];
    // Check if it is waiting for response
    // We can update the command status here
    // Or we could post an event to be handled on the ble_process_commands
    // It could be that we received more data
    switch (cmd->status) {
      case BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE: {
        if (length <= 0) break;
        temp_debug_print(data, length);
        if (is_expected_response(cmd, data, length)) {
          cmd->status = BLE_COMMAND_STATUS_OK;
          return 1;
        } else if (is_error_response(cmd, data, length)) {
          cmd->status = BLE_COMMAND_STATUS_ERROR;
          return 1;
        } else if (is_timedout(ble, cmd)) {
          cmd->status = BLE_COMMAND_STATUS_TIMEDOUT;
        }
        break;
      }
      default:;
    }

    if (length <= 0) return 1;
  }
  return 1;
}

int ble_process_commands(ble_t *ble) {
  if (head != tail) {
    ble_command_t *cmd = &commands[tail];
    switch (cmd->status) {
      case BLE_COMMAND_STATUS_PENDING: {
        ble->send_data((uint8_t *)cmd->data, strlen(cmd->data));
        if (strlen(cmd->response) > 0 && cmd->timeout_tick > 0) {
          cmd->status = BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE;
          cmd->tick = ble->get_tick();
        } else {
          cmd->status = BLE_COMMAND_STATUS_OK;
        }
        break;
      }
      case BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE: {
        // this is handled above
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

void ble_reset(ble_t *ble) {
  memset(ble, 0x00, sizeof(ble_t));
  ble->state = BLE_STATE_INIT;
}
