#ifndef BLE_PROTOCOL_COMMAND_H
#define BLE_PROTOCOL_COMMAND_H

#include <stdint.h>


int ble_protocol_command_ftms_data(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint16_t uuid);
int ble_protocol_heartrate_connecting(uint8_t *buffer, uint8_t *length, const uint8_t max_length);
int ble_protocol_heartrate_disconnect(uint8_t *buffer, uint8_t *length, const uint8_t max_length);
int ble_protocol_query_ftms_data_subscription(uint8_t *buffer, uint8_t *length, const uint8_t max_length);
int ftms_range_command(uint8_t *buffer, uint8_t *length, const uint8_t max_length,
                        const uint8_t range_id,
                        const uint16_t minimum, const uint16_t maximum, const uint16_t step);
int ble_protocol_debug_encode(uint8_t *buffer, uint8_t *length, const char *msg, uint8_t msg_len);

int build_hrm_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length, uint8_t subopcode);
int build_hrm_disconnect_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length);
int build_hrm_connect_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length);
int build_hrm_scan_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length);

int ble_protocol_command_create_rg_sn_hex_payload(uint8_t *buffer, const uint8_t max_length, uint32_t rg, uint64_t sn);

#endif
