#ifndef TREADMILL_MOTOR_MODBUS_H
#define TREADMILL_MOTOR_MODBUS_H

#include "treadmill.h"
#include "treadmill_motor.h"
#include "modbus/mb_operation.h"

int treadmill_motor_modbus_init(void);
int treadmill_motor_protocol_is_modbus(treadmill_t *treadmill);
treadmill_motor_status_t treadmill_motor_modbus_process(treadmill_t *treadmill);
int treadmill_set_next_mb_parameter(treadmill_t *treadmill);

uint16_t modbus_current_selected_param_value(void);
mb_operation_t *modbus_get_current_selected_param(void);

#endif
