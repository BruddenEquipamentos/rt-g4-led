#include "treadmill_ble.h"

#include <stdio.h>
#include <string.h>

#include "./ble/ble.h"
#include "./ble/ble_legacy.h"
#include "./ble/ble_protocol/ble_fmcp.h"
#include "./ble/ble_protocol/ble_ftms_treadmill.h"
#include "./ble/ble_protocol/ble_protocol_command.h"
#include "./ble/ble_protocol/ble_protocol_helper.h"

// TODO: refactor
#include <string.h>

#include "treadmill_action.h"

void treadmill_ble_process_init(treadmill_t *treadmill) {
  static uint32_t init_wait = 0;
  ble_t *ble = &treadmill->hw.ble;
  switch (ble->init_state) {
    case BLE_INIT_STATE_INIT: {
      if (ble->reset) ble->reset();
      ble->connected = 0;
      memset(&ble->ftms, 0x00, sizeof(struct ftms));
      memset(&ble->connection, 0x00, sizeof(struct connection));

      ble->init_state = BLE_INIT_STATE_WAIT;
      init_wait = treadmill->get_time_ms();
      break;
    }
    case BLE_INIT_STATE_WAIT: {
      if (treadmill->get_time_ms() - init_wait >= 2000) {
        init_wait = 0;
        ble->init_state = BLE_INIT_POST_COMMANDS;
      }
      break;
    }
    case BLE_INIT_POST_COMMANDS: {
      char data[32];
      uint8_t length;
      ble_clear_commands();
      ble_protocol_command_ftms_data((uint8_t *)data, &length, 32, FTMS_TREADMILL_DATA_UUID);
      ble_post_message(data, "\x06", 1000);

      ftms_range_command((uint8_t *)data, &length, 32, BP_FTMS_SUPPORTED_SPEED_RANGE, treadmill->config.speed.min,
                         treadmill->config.speed.max, treadmill->config.speed.step);
      ble_post_message(data, "\x06", 1000);

      ftms_range_command((uint8_t *)data, &length, 32, BP_FTMS_SUPPORTED_INCLINATION_RANGE,
                         treadmill->config.incline.min, treadmill->config.incline.max, treadmill->config.incline.step);
      ble_post_message(data, "\x06", 1000);
      ble_post_message(
          "\x02"
          "SID,10\n",
          "AOK", 1000);

      // TODO: name according to model
      snprintf(data, 32,
               "\x02"
               "SDN,%s\n",
               treadmill->config.device_name);
      ble_post_message(data, "AOK", 1000);

      extern uint16_t fw_version;
      snprintf(data, 32,
               "\x02"
               "SB1,%d\n",
               fw_version);
      ble_post_message(data, "AOK", 1000);

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

static int _tail = 0;
static int _head = 0;

enum ble_rx_event {
  BLE_RX_EVENT_NONE = 0,
  BLE_RX_EVENT_03_04_0A,
  BLE_RX_EVENT_F5_F6,
  BLE_RX_EVENT_CONNECTED,
  BLE_RX_EVENT_DISCONNECTED
};

#define BLE_DATA_SIZE 256
struct ble_data {
  enum ble_rx_event event;
  uint8_t length;
  uint8_t data[BLE_DATA_SIZE];
};

#define BLE_DATA_QUEUE_LENGTH (5)
struct ble_data ble_data[BLE_DATA_QUEUE_LENGTH];


void treadmill_ble_send_training_data(treadmill_t *treadmill) {
  treadmill->hw.ble.ftms.tick_data_ms = treadmill->get_time_ms();
  treadmill_send_ftms_data(treadmill);
}

static void treadmill_ble_reset(treadmill_t *treadmill) {
  memset(&treadmill->hw.ble.ftms, 0, sizeof(treadmill->hw.ble.ftms));
  memset(&treadmill->hw.ble.connection, 0, sizeof(treadmill->hw.ble.connection));
  memset(&treadmill->hw.ble.heartrate, 0, sizeof(treadmill->hw.ble.heartrate));
  memset(&treadmill->hw.ble.gk, 0, sizeof(treadmill->hw.ble.gk));
  memset(&treadmill->hw.ble.sh, 0, sizeof(treadmill->hw.ble.sh)); //Lucas
}

void treadmill_ble_process(treadmill_t *treadmill) {
  ble_t *ble = &treadmill->hw.ble;

#ifdef __APPLE__
  // receive message process
  uint8_t buffer[256];
  while (1) {
    volatile int length = ble->recv_data(buffer, 256);
    if (length <= 0) break;
    // ble_process_received(ble, received, length);
    for (int i = 0; i < length; ++i) {
      treadmill_ble_push_byte(treadmill, buffer[i]);
    }
  }
#endif

  while (_head != _tail) {
    struct ble_data *ble_data_msg = &ble_data[_tail];
    switch (ble_data_msg->event) {
      case BLE_RX_EVENT_03_04_0A: {
        if (!ble_legacy_commands_handler((char *)ble_data_msg->data)) {
          ble_process_received(&treadmill->hw.ble, (char *)ble_data_msg->data, ble_data_msg->length);
        }
        break;
      }
      case BLE_RX_EVENT_F5_F6: {
        ble_protocol_process(ble_data_msg->data, ble_data_msg->length);
        break;
      }
      case BLE_RX_EVENT_CONNECTED: {
        if (ble_data_msg->length > 0 && ble_data_msg->data[0] == 1) {
          treadmill->hw.ble.gk.connected = 1;
        }
        if (ble_data_msg->length > 0 && ble_data_msg->data[0] == 2) {
          treadmill->hw.ble.sh.connected = 1;
        }
        treadmill->hw.ble.connected = 1;
        break;
      }
      case BLE_RX_EVENT_DISCONNECTED: {        
        treadmill->hw.ble.connected = 0;
        treadmill_ble_reset(treadmill);
        break;
      }
      default:;
    }
    _tail = (_tail + 1) % BLE_DATA_QUEUE_LENGTH;
  }

  switch (ble->state) {
    case BLE_STATE_INIT: {
      treadmill_ble_process_init(treadmill);
      break;
    }
    case BLE_STATE_READY: {
      if (treadmill->hw.ble.ftms.subscribed) {      

        // To update each second 
        if (treadmill->scene != SCENE_TRAINING && treadmill->get_time_ms() - treadmill->hw.ble.ftms.tick_data_ms >= 1000) {
          treadmill_ble_send_training_data(treadmill);
        }
        
        // if (treadmill->get_time_ms() - treadmill->hw.ble.ftms.tick_data_ms >=
        //     (treadmill->hw.ble.connection.interval * 125) / 100) {
        //   treadmill->hw.ble.ftms.tick_data_ms = treadmill->get_time_ms();
        //   treadmill_send_ftms_data(treadmill);
        // }
      } else {
        // periodic queue command to check connection
        if (treadmill->get_time_ms() - treadmill->hw.ble.check_com_ms >= 10000) {
          treadmill->hw.ble.check_com_ms = treadmill->get_time_ms();
          ble_post_message(
              "\x02"
              "\n",
              "AOK", 1000);
        }
      }

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

void treadmill_ble_clear_messages(treadmill_t *treadmill) {
  memset(ble_data, 0x00, sizeof(struct ble_data));
  _head = 0;
  _tail = 0;
}
void treadmill_ble_post_message(enum ble_rx_event event, uint8_t *data, uint8_t length) {
  struct ble_data *ble_data_msg = &ble_data[_head];
  ble_data_msg->event = event;
  ble_data_msg->length = length;
  memset(ble_data_msg->data, 0x00, BLE_DATA_SIZE);
  memcpy(ble_data_msg->data, data, length);
  _head = (_head + 1) % BLE_DATA_QUEUE_LENGTH;
}

void treadmill_ble_push_byte(treadmill_t *treadmill, const uint8_t byte) {
#define RX_BUFFER_LENGTH (256)
  static uint8_t buffer[RX_BUFFER_LENGTH] = {0};
  static int length = 0;
  switch (byte) {
    case 0x01:
    case 0x02:
      length = 0;
      break;

    case 0x03:
    case 0x04:
    case 0x0A: {
      buffer[length] = 0;
      treadmill_ble_post_message(BLE_RX_EVENT_03_04_0A, buffer, length);
      break;
    }

    case 0xF0:
    case 0xF1:
      length = 0;
      buffer[length] = byte;
      length = (length + 1) % RX_BUFFER_LENGTH;
      break;

    case 0xF5:
    case 0xF6:
      buffer[length] = '\0';
      treadmill_ble_post_message(BLE_RX_EVENT_F5_F6, buffer, length);
      break;

    case 0x0F:
      treadmill_ble_post_message(BLE_RX_EVENT_CONNECTED, 0, 0);
      break;
    case 0x10: {
      static uint8_t gk = 1;
      treadmill_ble_post_message(BLE_RX_EVENT_CONNECTED, &gk, sizeof(gk));
      break;
    }
    case 0x11: {
    	static uint8_t sh = 2;
    	treadmill_ble_post_message(BLE_RX_EVENT_CONNECTED, &sh, sizeof(sh));
    	break;
    }
    case 0x19:
    case 0x0E:
      treadmill_ble_post_message(BLE_RX_EVENT_DISCONNECTED, 0, 0);
      break;

    // temp solution
    case 0x06:
    case 0x15:
      buffer[length] = byte;
      length = (length + 1) % RX_BUFFER_LENGTH;
      treadmill_ble_post_message(BLE_RX_EVENT_03_04_0A, buffer, length);
      break;

    default: {
      buffer[length] = byte;
      length = (length + 1) % RX_BUFFER_LENGTH;
    }
  }
}
