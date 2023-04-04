#ifndef BLE_SERIAL_H
#define BLE_SERIAL_H

#include <stdint.h>

int ble_serial_open(void);
int ble_serial_read(uint8_t *buffer, int max_length);
int ble_serial_write(const uint8_t *data, int length);

#endif
