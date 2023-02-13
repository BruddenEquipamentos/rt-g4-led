#include "treadmill_motor.h"

#include "treadmill_motor_modbus.h"
#include "treadmill_motor_movement.h"

static void treadmill_motor_update_frequency(treadmill_t *treadmill) {
  // todo: implement
}

static void treadmill_motor_uart_protocol_init(treadmill_t *treadmill) {
  treadmill_motor_modbus_init();
  treadmill->hw.protocol_state = UART_PROTOCOL_DETECTION_STATE;
}

static int treadmill_motor_uart_protocol_detection(treadmill_t *treadmill) {
  if (treadmill_motor_protocol_is_modbus(treadmill)) {
    treadmill->hw.protocol_state = UART_PROTOCOL_PROCESS_MODBUS_STATE;
    return 1;
  } else if (treadmill_motor_protocol_is_movement(treadmill)) {
    treadmill->hw.protocol_state = UART_PROTOCOL_PROCESS_MOVEMENT_STATE;
    return 1;
  }
  return 0;
}

static void treadmill_motor_update_uart(treadmill_t *treadmill) {
  //static int protocol_detection_error_count = 0;

  treadmill_motor_movement_process(treadmill);

  // switch (treadmill->hw.protocol_state) {
  //   case UART_PROTOCOL_INIT_STATE: {
  //     protocol_detection_error_count = 0;
  //     treadmill_motor_uart_protocol_init(treadmill);
  //     break;
  //   }
  //   case UART_PROTOCOL_DETECTION_STATE: {
  //     if (!treadmill_motor_uart_protocol_detection(treadmill)) {
  //       protocol_detection_error_count++;
  //       if (protocol_detection_error_count > 3) {
  //         treadmill->hw.protocol_state = UART_PROTOCOL_PROCESS_UNKNOWN;
  //       }
  //     }
  //     break;
  //   }
  //   case UART_PROTOCOL_PROCESS_MODBUS_STATE: {
  //     treadmill_motor_modbus_process(treadmill);
  //     break;
  //   }
  //   case UART_PROTOCOL_PROCESS_UNKNOWN:
  //   case UART_PROTOCOL_PROCESS_MOVEMENT_STATE: {
  //     treadmill_motor_movement_process(treadmill);
  //     break;
  //   }
  // }
}

#include <string.h>

#include "debug.h"

// #define MTEST

void treadmill_motor_process(treadmill_t *treadmill) {
#ifdef MTEST
  static uint32_t test_tick = 0;
  static int op = 0;
  static uint8_t buffer[256] = {0};
  if (treadmill->get_time_ms() - test_tick >= 1000) {
    test_tick = treadmill->get_time_ms();
    switch (op) {
      case 0: {
        op = 1;
        treadmill->hw.motor.send((uint8_t *)"Hello,World",
                                 strlen("Hello,World"));
        break;
      }
      case 1: {
        op = 0;
        memset(buffer, 0x00, 256);
        int length = treadmill->hw.motor.recv(buffer, 256);
        if (length == 0) {
          DPRINT("No Data\n");
        } else {
          DPRINT("Got Data %d: %s\n", length, (char *)buffer);
        }
        break;
      }
    }
  }
#else
  treadmill_motor_update_uart(treadmill);

  // switch (treadmill->hw.communication_type) {
  //   case MOTOR_COMMUNICATION_TYPE_FREQ: {
  //     treadmill_motor_update_frequency(treadmill);
  //     break;
  //   }
  //   case MOTOR_COMMUNICATION_TYPE_UART: {
  //     treadmill_motor_update_uart(treadmill);
  //     break;
  //   }
  //   default:;
  // }
#endif
}
