#include "treadmill_ble.h"

#include "./ble/ble.h"
#include "./ble/ble_protocol/ble_fmcp.h"
#include "./ble/ble_protocol/ble_ftms_treadmill.h"
#include "./ble/ble_protocol/ble_protocol_command.h"
#include "./ble/ble_protocol/ble_protocol_helper.h"

void treadmill_ble_process_init(treadmill_t *treadmill) {
  ble_t *ble = &treadmill->hw.ble;
  switch (ble->init_state) {
    case BLE_INIT_STATE_INIT: {
      char data[32];
      uint8_t length;
      ble_clear_commands();
      ble_protocol_command_ftms_data((uint8_t *)data, &length, 32,
                                     FTMS_TREADMILL_DATA_UUID);
      ble_post_message(data, "\x06", 1000);

      ftms_range_command(
          (uint8_t *)data, &length, 32, BP_FTMS_SUPPORTED_SPEED_RANGE,
          treadmill->config.speed.min, treadmill->config.speed.max,
          treadmill->config.speed.step);
      ble_post_message(data, "\x06", 1000);

      ftms_range_command(
          (uint8_t *)data, &length, 32, BP_FTMS_SUPPORTED_INCLINATION_RANGE,
          treadmill->config.incline.min, treadmill->config.incline.max,
          treadmill->config.incline.step);
      ble_post_message(data, "\x06", 1000);
      ble_post_message("\x02SID,2\n", "AOK", 1000);
      ble_post_message("\x02SDN,MovCT\n", "AOK", 1000);
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

void treadmill_ble_process(treadmill_t *treadmill) {
  // MACRO VIEW OF THE BLE PROCESS

  // READ MESSAGES -> CHECK POSTED MESSAGE RESPONSE | INTERPRET MESSAGES
  // POST MESSAGE -> SEND MESSAGE

  ble_t *ble = &treadmill->hw.ble;

#define RX_MAX_LENGTH (256)
  static char received[RX_MAX_LENGTH] = {0};

  // receive message process
  while (1) {
    int length = ble->recv_data(
        (uint8_t *)received, RX_MAX_LENGTH);  // we assume that recv_data will
                                              // return on parse a command
    if (length <= 0) break;
    // handle reponse here
    ble_process_received(ble, received, length);
  }

  switch (ble->state) {
    case BLE_STATE_INIT: {
      treadmill_ble_process_init(treadmill);
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
