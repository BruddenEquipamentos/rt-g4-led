#ifndef TREADMILL_MOTOR_MOVEMENT_H
#define TREADMILL_MOTOR_MOVEMENT_H

#include "treadmill.h"
#include "treadmill_motor.h"

int treadmill_motor_protocol_is_movement(treadmill_t *treadmill);
treadmill_motor_status_t treadmill_motor_movement_process(
    treadmill_t *treadmill);

#endif
