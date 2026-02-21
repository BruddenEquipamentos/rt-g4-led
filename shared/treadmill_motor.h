#ifndef TREADMILL_MOTOR_H
#define TREADMILL_MOTOR_H

#include "treadmill.h"

typedef enum treadmill_motor_status {
  TREADMILL_MOTOR_STATUS_ERROR = 0,
  TREADMILL_MOTOR_STATUS_OK,
} treadmill_motor_status_t;

void treadmill_motor_process(treadmill_t *treadmill);
void treadmill_motor_register_inverter_error(treadmill_t *treadmill, uint8_t error_code);

#endif
