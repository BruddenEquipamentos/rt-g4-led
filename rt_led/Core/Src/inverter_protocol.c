#include "inverter_protocol.h"

#include "cmsis_os.h"
#include "modus/mb_master.h"
#include "modus/mb_operation.h"

static enum serial_state serial_state = SERIAL_PROTOCOL_INIT;
static enum serial_protocol_id serial_protocol_id = SERIAL_PROTOCOL_UNKNOW;

static uint8_t bytes_4[4] = {0};
mb_operation_t *p023 = 0;
mb_operation_t *p682 = 0;
mb_operation_t *p683 = 0;
mb_operation_t *p856 = 0;
mb_operation_t *p1000_1001 = 0;

static int protocol_pattern_id_modbus_rtu() {
  // lets start modbus operation and see if the response is valid
  // send  at least 3 commands and  expect at least 3 valid responses
  // we can start transmiting from the modbus ring
  // we need to achieve 3 valid responses 6 frames
  enum mb_master_process status;
  for (int i = 0; i < 6; ++i) {
    status = mb_operation_process(p023);
    if (status == MB_MASTER_OK) return 1;
  }
  return 0;
}

static int protocol_pattern_id_movement() {
  // note(fminoru):the idea for detection is to send 3 valid commands and expect
  // the ACK byte; For the future, we could abstract the serial reception.
  uint8_t acks = 0;
  static uint8_t commands[3][3] = {
      {0x02, 0x41, 0x00},
      {0x02, 0x42, 0x00},
      {0x02, 0x43, 0x00},
  };
  for (int i = 0; i < 9; ++i) {
    if (!serial_send(commands[i % 3], 3)) return 0;
    if (xSemaphoreTake(xSemaphoreRxACK, 10) == pdTRUE) acks++;
    if (acks >= 3) return 1;
  }
  return 0;
}

static enum serial_protocol_id serial_protocol_detection(
    enum serial_state *state) {
  char attempts = 2;
  while (attempts--) {
    if (protocol_pattern_id_modbus_rtu()) {
      *state = SERIAL_PROTOCOL_PROCESS;
      return SERIAL_PROTOCOL_MODBUS_RTU;
    }
    osDelay(500);
    if (protocol_pattern_id_movement()) {
      *state = SERIAL_PROTOCOL_PROCESS;
      return SERIAL_PROTOCOL_MOVEMENT;
    }
    osDelay(500);
  }
  *state = SERIAL_PROTOCOL_PROCESS;
  return SERIAL_PROTOCOL_UNKNOW;
}

static void serial_process(void) {
  static uint8_t err_count = 0;
  switch (serial_state) {
    case SERIAL_PROTOCOL_INIT:
      serial_state = SERIAL_PROTOCOL_DETECTION;
      break;

    case SERIAL_PROTOCOL_DETECTION:
      serial_protocol_id = serial_protocol_detection(&serial_state);
      break;

    case SERIAL_PROTOCOL_PROCESS:
      switch (serial_protocol_id) {
        case SERIAL_PROTOCOL_UNKNOW:
          movement_process(xThis, pxVisSts);
          osDelay(10);
          break;

        case SERIAL_PROTOCOL_MOVEMENT: {
          // movement_process(xThis, pxVisSts);
          if (movement_process(xThis, pxVisSts))
            err_count = 0;
          else
            err_count++;
          if (err_count >= 12) {
            err_count = 0;
            serial_protocol_id = SERIAL_PROTOCOL_UNKNOW;
            serial_state = SERIAL_PROTOCOL_DETECTION;
          } else {
            osDelay(10);
          }
          break;
        }
        case SERIAL_PROTOCOL_MODBUS_RTU: {
          if (modbus_process(xThis, pxVisSts) == MB_MASTER_OK)
            err_count = 0;
          else
            err_count++;

          if (err_count >= 12) {
            err_count = 0;
            serial_state = SERIAL_PROTOCOL_DETECTION;
            serial_protocol_id = SERIAL_PROTOCOL_UNKNOW;
            mb_operation_reenable_run_once();
          }
          break;
        }
      }
      break;
    default:
      serial_state = SERIAL_PROTOCOL_INIT;
  }
}
