#include "ble.h"

#include <string.h>

#include "../debug.h"
#include "./ble_protocol/ble_fmcp.h"
#include "./ble_protocol/ble_ftms_treadmill.h"
#include "./ble_protocol/ble_protocol_helper.h"
#include "ble_legacy.h"

// TEMP
#include "../treadmill.h"
#include "../treadmill_action.h"
extern treadmill_t *pTreadmill;

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

static int ble_ftms_control_point(char *ptr) {
  fmcp_result_t result = FMCP_RESULT_SUCCESS;
  uint8_t opcode = next_u8(&ptr);
  switch (opcode) {
    case FMCP_REQUEST_CONTROL: {
      // todo: rule to allow control point control
      pTreadmill->ble.ftms_cp_allowed = 1;
      break;
    }
    case FMCP_RESET: {
      // todo: implement
      break;
    }
    case FMCP_SET_TARGET_SPEED: {
      uint16_t speed = next_u16_le(&ptr);
      if (!treadmill_set_speed(pTreadmill, speed))
        result = FMCP_RESULT_CODE_OPERATION_FAILED;
      break;
    }
    case FMCP_SET_TARGET_INCLINATION: {
      int16_t inclination = next_i16_le(&ptr);
      if (!treadmill_set_incline(pTreadmill, inclination))
        result = FMCP_RESULT_CODE_OPERATION_FAILED;
      break;
    }
    case FMCP_SET_TARGET_START_OR_RESUME: {
      if (pTreadmill->ble.ftms_cp_allowed) {
        if (!treadmill_start_or_resume(pTreadmill))
          result = FMCP_RESULT_CODE_OPERATION_FAILED;
      } else {
        result = FMCP_RESULT_CODE_CONTROL_NOT_PERMITTED;
      }
      break;
    }

    case FMCP_SET_TARGET_STOP_OR_PAUSE: {
      uint8_t param = next_u8(&ptr);
      if (param == 1) {
        if (!treadmill_stop_procedure(pTreadmill))
          result = FMCP_RESULT_CODE_OPERATION_FAILED;
      } else if (param == 2) {
        if (!treadmill_pause_procedure(pTreadmill))
          result = FMCP_RESULT_CODE_OPERATION_FAILED;
      }
      break;
    }

    default:
      result = FMCP_RESULT_CODE_OPCODE_NOT_SUPPORTED;
      break;
  }

  // return movble_treadmill_ftms_control_point_response(FMCP_RESPONSE_CODE,
  // opcode, result);
  return result;
}

static void ble_protocol_process(uint8_t *ptr_data, int length) {
  if (!ble_protocol_valid_message(ptr_data, length)) {
    return;
  }

  char *ptr = (char *)&ptr_data[1];
  uint8_t opcode = next_u8(&ptr);

  switch (opcode) {
    case BP_SUBSCRIBE: {
      uint16_t uuid = next_u16_le(&ptr);
      if (uuid == FTMS_TREADMILL_DATA_UUID) {
        pTreadmill->ble.ftms_subscribed = 1;
      }
      // movble_treadmill_subscribed(ptr);
      break;
    }
    case BP_UNSUBSCRIBE: {
      uint16_t uuid = next_u16_le(&ptr);
      if (uuid == FTMS_TREADMILL_DATA_UUID) {
        pTreadmill->ble.ftms_subscribed = 0;
      }
      break;
    }
    case BP_CONTROL_POINT: {
      ble_ftms_control_point(ptr);
      break;
    }
    case BP_LINKINFO: {
      pTreadmill->ble.connection.interval = next_u16_le(&ptr);
      pTreadmill->ble.connection.latency = next_u16_le(&ptr);
      pTreadmill->ble.connection.timeout = next_u16_le(&ptr);
      pTreadmill->ble.connection.mtu = next_u16_le(&ptr);
      break;
    }
    case BP_BLE_HEARTRATE_CONNECTING: {
      pTreadmill->ble.heartrate.connecting = 1;
      break;
    }
    case BP_BLE_HEARTRATE: {
      pTreadmill->ble.heartrate.connecting = 0;
      pTreadmill->ble.heartrate.bpm = next_u8(&ptr);
      break;
    }
    case BP_BLE_HEARTRATE_DISCONNECT: {
      pTreadmill->ble.heartrate.connecting = 0;
      break;
    }
    default:;
  }
}

static void ble_connected(ble_t *ble) { ble->connected = 1; }

static void ble_disconnected(ble_t *ble) {
  ble->connected = 0;
  ble->ftms_cp_allowed = 0;
  ble->ftms_subscribed = 0;
  ble->connection.interval = 0;
  ble->connection.latency = 0;
  ble->connection.timeout = 0;
  ble->connection.mtu = 0;
}

static void ble_interpret_message_bytes(ble_t *ble, char *data, int length) {
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
        ble_legacy_commands_handler((char *)mBuffer);
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

      case 0x0F:
        ble_connected(ble);
        break;

      case 0x19:
      case 0x0E:
        ble_disconnected(ble);
        break;

      default: {
        mBuffer[mBufferIndex++] = byte;
      }
    }
  }
}

int ble_process_received(ble_t *ble, char *data, int length) {
  ble_interpret_message_bytes(ble, data, length);

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
