#ifndef PARSER_H
#define PARSER_H

#define PARSER_HELPER_LITTLE_ENDIAN
/* #define PARSER_HELPER_BIG_ENDIAN */

#include <stdint.h>

typedef enum BleProtocolOpcode {
  BP_PROGRAMMING_START = 0x00,
  BP_PROGRAMMING_INTERVAL = 0x01,
  BP_PROGRAMMING_FINISH = 0x02,
  BP_PROGRAMMING_CLEAR = 0x03,

  BP_SUBSCRIBE = 0x04,
  BP_UNSUBSCRIBE = 0x05,

  BP_INDICATE = 0x06,
  BP_NOTIFY = 0x07,

  BP_CONTROL_POINT = 0x08,

  BP_LINKINFO = 0x09,

  BP_FTMS_DATA = 0x0A,
  BP_QUERY_SUBSCRIBED_FTMS_DATA = 0x0B,

  // BLE Heart Rate Status and Data (OLD HRM)
  BP_BLE_HEARTRATE_CONNECTING = 0x0C,
  BP_BLE_HEARTRATE = 0x0D,
  BP_BLE_HEARTRATE_DISCONNECT = 0x0E,

	BP_FTMS_SUPPORTED_SPEED_RANGE= 0x0F,
	BP_FTMS_SUPPORTED_INCLINATION_RANGE = 0x10,
	BP_FTMS_SUPPORTED_RESISTANCE_LEVEL_RANGE = 0x11,

	BP_GYMKIT_HEART_RATE_DISABLED,
	BP_GYMKIT_HEART_RATE_UNAVAILABLE,
	BP_GYMKIT_HEART_RATE_DATA,

  BP_FMS_NOTIFY = 0x20,
  BP_GK_NOTIFICATION = 0x21,

  BP_BLE_HRM = 0x22, // (NEW HRM)

  BP_MANUFACTURER_DATA = 0x23,

  BP_DEBUG = 0x2A
} ble_protocol_opcode_t;

// BLE_HR,1 -> BLE_HRM_SCAN
// BLE_HR,0 -> BLE_HRM_DEVICE_NAME
typedef enum ble_protocol_heart_rate_opcode {  
  BP_BLE_HRM_SCAN = 0x00,  // <- request from panel
  BP_BLE_HRM_DEVICE_NAME,  // -> notification to panel (length 1B, name B)
  BP_BLE_HRM_CONNECT,      // <- request from panel
  BP_BLE_HRM_CONNECTING,   // -> notification to panel
  BP_BLE_HRM_CONNECTED,    // -> notification to panel
  BP_BLE_HRM_NOTIFICATION, // -> notification to panel (heart rate 1B)
  BP_BLE_HRM_DISCONNECT,   // <- request from panel
  BP_BLE_HRM_DISCONNECTED, // -> notification to panel
} ble_protocol_heart_rate_opcode_t;

typedef enum ble_protocol_control_byte {
  BLE_PROTOCOL_CONNECT_BYTE = 0x0F,
  BLE_PROTOCOL_CONNECT_GK_BYTE = 0x10,
  
  BLE_PROTOCOL_DISCONNECT_BYTE = 0x19,
  BLE_PROTOCOL_SOH = 0x01,
  BLE_PROTOCOL_EOT = 0x04,
  BLE_PROTOCOL_STX = 0x02,
  BLE_PROTOCOL_ETX = 0x03,
  BLE_PROTOCOL_MESSAGE_START = 0xF1,
  BLE_PROTOCOL_MESSAGE_END = 0xF5,
} ble_protocol_control_byte_t;

typedef enum BleFrameType {
    FRAME_TYPE_DATA = 0 << 6,    // 00: Data frame  (0x00)
    FRAME_TYPE_START = 1 << 6,   // 01: Start frame (0x40)
    FRAME_TYPE_END = 2 << 6,     // 10: End frame   (0x80)
    FRAME_TYPE_SINGLE = 3 << 6   // 11: Single frame (0xC0)
} ble_frame_type_t;

// Define IDs for the BleFramePacket data union
#define FRAME_ID_RG_SN 0x01
#define FRAME_ID_COMPONENT_DATA 0x02
// Add other frame data IDs as needed

// Ensure the struct is packed to prevent compiler padding.
// __attribute__((packed)) is for GCC/Clang. Use #pragma pack(1) for MSVC.
typedef struct __attribute__((packed)) BleFramePacket {        
    struct __attribute__((packed)) { // Also pack nested struct
        uint8_t frame_index;
        uint8_t frame_length;
    } metadata;

    struct __attribute__((packed)) { // Also pack nested struct
        uint8_t id;
        union __attribute__((packed)) { // Also pack the union itself
            struct __attribute__((packed)) { // And structs within the union
                uint8_t rg[3];
                uint8_t sn[7];
            };

            struct __attribute__((packed)) { // And structs within the union
                uint16_t component_id;
                uint32_t distance;
                uint32_t time;
            };
        } data;
    } frame_data;
} ble_frame_packet_t;

uint8_t read_u8(char *ptr);
uint8_t next_u8(char **ptr);
int8_t next_i8(char **ptr);

#ifdef PARSER_HELPER_LITTLE_ENDIAN
uint16_t read_u16_le(char *ptr);
uint16_t next_u16_le(char **ptr);
int16_t next_i16_le(char **ptr);
uint32_t next_u24_le(char **ptr);
uint32_t next_u32_le(char **ptr);
#endif

#ifdef PARSER_HELPER_BIG_ENDIAN
uint16_t read_u16_be(char *ptr);
int16_t next_i16_be(char **ptr);
uint16_t next_u16_be(char **ptr);
uint32_t next_u24_be(char **ptr);
uint32_t next_u32_ne(char **ptr);
#endif

char hex_from_nibble(uint8_t nibble);
char hex_nibble_high(uint8_t value);
char hex_nibble_low(uint8_t value);

int ble_protocol_valid_message(const uint8_t *buffer, const int length);
uint8_t ble_protocol_checksum(const uint8_t *data, const uint8_t length);

int flat_bin_to_ascii_24(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint32_t data);
int flat_bin_to_ascii_16(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint16_t data);
int flat_bin_to_ascii_8(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint8_t data);

// Helper functions for BleFramePacket
void ble_frame_packet_init_rg_sn(ble_frame_packet_t *packet, uint8_t frame_index, uint32_t rg, uint64_t sn);
void ble_frame_packet_init_component_data(ble_frame_packet_t *packet, uint8_t frame_index, uint16_t component_id, uint32_t distance, uint32_t time);

// Generic encoder: encodes the raw bytes of the (packed and endian-corrected) struct.
int ble_frame_packet_generic_encode_to_hex(const ble_frame_packet_t *packet, uint8_t *output_buffer, uint8_t *current_pos, const uint8_t max_output_length);
#endif
