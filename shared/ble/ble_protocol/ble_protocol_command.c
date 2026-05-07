#include "ble_protocol_command.h"
#include "ble_protocol_helper.h"

#include <string.h>

int ble_protocol_command_ftms_data(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint16_t uuid) {
    *length = 0;
    for (int i = 0; i < max_length; ++i) buffer[i] = 0x00;
    buffer[(*length)++] = 0xF1;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_FTMS_DATA)) return 0;
    if (!flat_bin_to_ascii_16(buffer, length, max_length, uuid)) return 0;
    int cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs)) return 0;
    buffer[(*length)++] = 0xF5;
    return 1;
}
int ble_protocol_query_ftms_data_subscription(uint8_t *buffer, uint8_t *length, const uint8_t max_length) {
    *length = 0;
    for (int i = 0; i < max_length; ++i) buffer[i] = 0x00;
    buffer[(*length)++] = 0xF1;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_QUERY_SUBSCRIBED_FTMS_DATA)) return 0;
    int cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs)) return 0;
    buffer[(*length)++] = 0xF5;
    return 1;
}

// TODO: Refactor (used only in CC2642 BLE Module (BLE Module -> Panel))
int ble_protocol_heartrate_connecting(uint8_t *buffer, uint8_t *length, const uint8_t max_length) {
    *length = 0;
    for (int i = 0; i < max_length; ++i) buffer[i] = 0x00;
    buffer[(*length)++] = 0xf1;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HEARTRATE_CONNECTING)) return 0;
    uint8_t cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs)) return 0;
    buffer[(*length)++] = 0xf5;
    return 1;
}

int ble_protocol_heartrate_disconnect(uint8_t *buffer, uint8_t *length, const uint8_t max_length) {
    *length = 0;
    for (int i = 0; i < max_length; ++i) buffer[i] = 0x00;
    buffer[(*length)++] = 0xf1;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HEARTRATE_DISCONNECT)) return 0;
    uint8_t cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs)) return 0;
    buffer[(*length)++] = 0xf5;
    return 1;
}

int ftms_range_command(uint8_t *buffer, uint8_t *length, const uint8_t max_length,
                        const uint8_t range_id,
                        const uint16_t minimum, const uint16_t maximum, const uint16_t step) {
    *length = 0;
    for (int i = 0; i < max_length; ++i) buffer[i] = 0x00;
    buffer[(*length)++] = 0xF1;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, range_id)) return 0;
    if (!flat_bin_to_ascii_16(buffer, length, max_length, minimum)) return 0;
    if (!flat_bin_to_ascii_16(buffer, length, max_length, maximum)) return 0;
    if (!flat_bin_to_ascii_16(buffer, length, max_length, step)) return 0;
    int cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs)) return 0;
    buffer[(*length)++] = 0xF5;
    return 1;
}

int ble_protocol_debug_encode(uint8_t *buffer, uint8_t *length, const char *msg, uint8_t msg_len) {
    *length = 0;
    for (int i = 0; i < msg_len + 5; ++i) buffer[i] = 0x00;
    
    buffer[(*length)++] = 0xF1;
    if (!flat_bin_to_ascii_8(buffer, length, msg_len + 5, BP_DEBUG)) return 0;
    
    // Copy message directly since it's a string
    memcpy(&buffer[*length], msg, msg_len);
    *length += msg_len;
    
    int cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, msg_len + 5, cs)) return 0;
    
    buffer[(*length)++] = 0xF5;
    return 1;
}

//Lucas (from Francis) - New commands - BLE HRM

int build_hrm_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length, uint8_t subopcode) {
    *length = 0;
    for (int i = 0; i < max_length; ++i) buffer[i] = 0x00;    
    buffer[(*length)++] = 0xF1;  // Start of message    
    // First write BP_BLE_HRM opcode
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HRM)) return 0;
    // Then write BP_BLE_HRM_SCAN sub-opcode
    if (!flat_bin_to_ascii_8(buffer, length, max_length, subopcode)) return 0;
    int cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs)) return 0;
    
    buffer[(*length)++] = 0xF5;  // End of message
    return 1;
}

int build_hrm_disconnect_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length) {
    return build_hrm_packet(buffer, length, max_length, BP_BLE_HRM_DISCONNECT);
}

int build_hrm_connect_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length) {
    return build_hrm_packet(buffer, length, max_length, BP_BLE_HRM_CONNECT);
}

int build_hrm_scan_packet(uint8_t *buffer, uint8_t *length, const uint8_t max_length) {
    return build_hrm_packet(buffer, length, max_length, BP_BLE_HRM_SCAN);
}
