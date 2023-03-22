#ifndef TREADMILL_BLE_H
#define TREADMILL_BLE_H

#include "treadmill.h"

void treadmill_ble_process(treadmill_t *treadmill);
void treadmill_ble_push_byte(treadmill_t *treadmill, const uint8_t byte);

#endif
