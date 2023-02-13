#ifndef TREADMILL_MOTOR_MODBUS_H
#define TREADMILL_MOTOR_MODBUS_H

#include "treadmill.h"
#include "treadmill_motor.h"

int treadmill_motor_modbus_init(void);
int treadmill_motor_protocol_is_modbus(treadmill_t *treadmill);
treadmill_motor_status_t treadmill_motor_modbus_process(treadmill_t *treadmill);

#endif
