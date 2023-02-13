#ifndef MOTOR_SERIAL_H
#define MOTOR_SERIAL_H

#include <stdint.h>

int motor_serial_open();
int motor_serial_read(uint8_t *buffer, int max_length);
int motor_serial_write(const uint8_t *data, int length);

#endif
