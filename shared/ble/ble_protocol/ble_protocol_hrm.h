#ifndef BLE_PROTOCOL_HRM_H
#define BLE_PROTOCOL_HRM_H

#include <stdint.h>
#include "ble_protocol_helper.h"

// Function to encode HRM device name message (sent to panel)
// Returns 1 on success, 0 on failure
int ble_protocol_hrm_encode_device_name(uint8_t *buffer, uint8_t *length, 
                                      const uint8_t max_length, 
                                      const char *device_name);

// Function to encode HRM connecting message (sent to panel)
int ble_protocol_hrm_encode_connecting(uint8_t *buffer, uint8_t *length,
                                     const uint8_t max_length);

// Function to encode HRM connected message (sent to panel)  
int ble_protocol_hrm_encode_connected(uint8_t *buffer, uint8_t *length,
                                    const uint8_t max_length);

// Function to encode HRM notification message (sent to panel)
int ble_protocol_hrm_encode_notification(uint8_t *buffer, uint8_t *length,
                                       const uint8_t max_length,
                                       const uint8_t heart_rate);

// Function to decode received HRM message
// Returns opcode if valid message, -1 if invalid
int ble_protocol_hrm_decode(const uint8_t *buffer, const uint8_t length,
                           ble_protocol_heart_rate_opcode_t *opcode);

#endif 