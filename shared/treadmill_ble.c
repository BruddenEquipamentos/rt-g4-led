#include "treadmill_ble.h"
#include <string.h>

static void treadmill_ble_set_connected(treadmill_t *treadmill) {
  treadmill->ble.connected = 1;
}

static void treadmill_ble_set_disconnected(treadmill_t *treadmill) {
  treadmill->ble.connected = 0;
}

static inline void treadmill_ble_push_byte(treadmill_t *treadmill,
                                           const uint8_t byte) {
  #define BUFFER_SIZE (256)
  static uint8_t buffer[BUFFER_SIZE];
  static int index;

  switch (byte) {
    case 0x0f:
      treadmill_ble_set_connected(treadmill);
      break;
    case 0x19:
      treadmill_ble_set_disconnected(treadmill);
      break;
    case 0x01:
      index = 0;
      break;
    case 0x04:
      // handle as ble serial
      break;
    case 0x02:
      index = 0;
      memset(buffer, 0x00, BUFFER_SIZE);
      break;
    case 0x03:
    case 0x0A:
      // todo: archive responde data

      /*
      elliptical->ble.command.response.length = (rxindex > 32) ? 32 : rxindex;
      memcpy(elliptical->ble.command.response.data, rxbuffer,
             elliptical->ble.command.response.length);
      */

      index = 0;

      // todo: handle response data
      /*
      if (strncmp((char *)rxbuffer, "AOK", rxindex) == 0) {
        elliptical->ble.command.response.code = BLE_RESPONSE_ACK;
      } else if (strncmp((char *)rxbuffer, "ERR", rxindex)) {
        elliptical->ble.command.response.code = BLE_RESPONSE_NAK;
      } else {
        elliptical->ble.command.response.code = BLE_RESPONSE_DATA;
      }
      */
      break;

    case 0x06:
      // todo: register response ACK

      // elliptical->ble.command.response.code = BLE_RESPONSE_ACK;
      break;

    case 0x15:
      // todo: register response NAK

      // elliptical->ble.command.response.code = BLE_RESPONSE_NAK;
      break;

    case 0xf0:
    case 0xf1:
      index = 0;
      buffer[index++] = byte;
      break;

    case 0xf5:
    case 0xf6:
      buffer[index] = '\0';
      // todo: process message routine
      // xe_ble_process_message(elliptical, rxbuffer, rxindex);
      index = 0;
      break;

    default:
      if (index >= BUFFER_SIZE) index = 0;
      buffer[index++] = byte;
      ;
  }
}

int treadmill_ble_input_data(treadmill_t *treadmill, const char *data,
                             const uint32_t length) {
  for (uint32_t i = 0; i < length; ++i) {
    treadmill_ble_push_byte(treadmill, data[i]);
  }
  return 1;
}
