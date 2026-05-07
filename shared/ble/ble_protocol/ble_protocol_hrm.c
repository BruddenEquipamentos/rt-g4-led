#include "ble_protocol_hrm.h"
#include <string.h>

static int encode_hrm_message(uint8_t *buffer, uint8_t *length,
                            const uint8_t max_length,
                            const ble_protocol_heart_rate_opcode_t opcode) {
    *length = 0;
    buffer[(*length)++] = 0xF1;
    
    // Encode main opcode (BP_BLE_HRM)
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HRM)) 
        return 0;
        
    // Encode HRM sub-opcode
    if (!flat_bin_to_ascii_8(buffer, length, max_length, opcode))
        return 0;
        
    // Calculate and encode checksum
    uint8_t cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs))
        return 0;
        
    buffer[(*length)++] = 0xF5;
    return 1;
}

int ble_protocol_hrm_encode_device_name(uint8_t *buffer, uint8_t *length,
                                      const uint8_t max_length,
                                      const char *device_name) {
    *length = 0;
    buffer[(*length)++] = 0xF1;
    
    // Encode main opcode
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HRM))
        return 0;
        
    // Encode sub-opcode
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HRM_DEVICE_NAME))
        return 0;
        
    // Encode device name length
    size_t name_len = strlen(device_name);
    if (name_len > 255) name_len = 255; // Limit to max uint8_t value
    if (!flat_bin_to_ascii_8(buffer, length, max_length, (uint8_t)name_len))
        return 0;
    
    // Encode device name using flat_bin_to_ascii_8 for each byte
    for (size_t i = 0; i < name_len; i++) {
        if (!flat_bin_to_ascii_8(buffer, length, max_length, (uint8_t)device_name[i]))
            return 0;
    }
    
    // Checksum and end marker
    uint8_t cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs))
        return 0;
        
    buffer[(*length)++] = 0xF5;
    return 1;
}

int ble_protocol_hrm_encode_connecting(uint8_t *buffer, uint8_t *length,
                                     const uint8_t max_length) {
    return encode_hrm_message(buffer, length, max_length, BP_BLE_HRM_CONNECTING);
}

int ble_protocol_hrm_encode_connected(uint8_t *buffer, uint8_t *length,
                                    const uint8_t max_length) {
    return encode_hrm_message(buffer, length, max_length, BP_BLE_HRM_CONNECTED);
}

int ble_protocol_hrm_encode_notification(uint8_t *buffer, uint8_t *length,
                                       const uint8_t max_length,
                                       const uint8_t heart_rate) {
    *length = 0;
    buffer[(*length)++] = 0xF1;
    
    // Encode main opcode
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HRM))
        return 0;
        
    // Encode sub-opcode
    if (!flat_bin_to_ascii_8(buffer, length, max_length, BP_BLE_HRM_NOTIFICATION))
        return 0;
        
    // Encode heart rate value
    if (!flat_bin_to_ascii_8(buffer, length, max_length, heart_rate))
        return 0;
        
    // Checksum and end marker
    uint8_t cs = ble_protocol_checksum(buffer, *length);
    if (!flat_bin_to_ascii_8(buffer, length, max_length, cs))
        return 0;
        
    buffer[(*length)++] = 0xF5;
    return 1;
}

int ble_protocol_hrm_decode(const uint8_t *buffer, const uint8_t length,
                           ble_protocol_heart_rate_opcode_t *opcode) {
    // Check minimum length and start/end markers
    if (length < 7 || buffer[0] != 0xF1 || buffer[length-1] != 0xF5)
        return -1;
        
    // Verify checksum
    if (!ble_protocol_valid_message(buffer, length))
        return -1;
        
    char *ptr = (char *)buffer + 1;
    
    // Check main opcode
    uint8_t main_opcode = next_u8(&ptr);
    if (main_opcode != BP_BLE_HRM)
        return -1;
        
    // Get sub-opcode
    *opcode = (ble_protocol_heart_rate_opcode_t)next_u8(&ptr);
    
    return 0;
} 
