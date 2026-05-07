#include "ble_protocol_helper.h"
#include <string.h> // For memcpy, if needed, though not directly for this change

static uint16_t ble_protocol_htons(uint16_t value) {
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return value;
#else
    return (uint16_t)((value << 8) | (value >> 8));
#endif
}

static inline uint8_t read_value(const char ch) {
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return ch - '0';
}

uint8_t read_u8(char *ptr) {
    uint8_t nh = read_value(*ptr);
    uint8_t nl = read_value(*(ptr+1));
    *ptr = (*ptr) + 2;
    return (nh << 4) | nl;
}

uint8_t next_u8(char **ptr) {
    uint8_t value = read_u8(*ptr);
    *ptr = (*ptr) + 2;
    return value;
}

int8_t next_i8(char **ptr) {
    return (int8_t)next_u8(ptr);
}

inline char hex_from_nibble(uint8_t nibble) {
    if (nibble >= 10) return (nibble%10) + 'A';
    return '0' + nibble;
}

inline char hex_nibble_high(uint8_t value) {
    value = (value >> 4);
    return hex_from_nibble(value);
}

inline char hex_nibble_low(uint8_t value) {
    value = value & 0x0f;
    return hex_from_nibble(value);
}

#ifdef PARSER_HELPER_LITTLE_ENDIAN
uint16_t read_u16_le(char *ptr) {
    uint8_t lo = read_u8(ptr);
    uint8_t hi = read_u8(ptr + 2);
    return (hi << 8) | lo;
}

uint16_t next_u16_le(char **ptr) {
    uint8_t lo = next_u8(ptr);
    uint8_t hi = next_u8(ptr);
    return (hi << 8) | (lo & 0xff);
}

int16_t next_i16_le(char **ptr) {
    return (int16_t) next_u16_le(ptr);
}

uint32_t next_u24_le(char **ptr) {
    uint8_t lo = next_u8(ptr);
    uint8_t me = next_u8(ptr);
    uint8_t hi = next_u8(ptr);
    return (hi << 16) | (me << 8) | lo;
}

uint32_t next_u32_le(char **ptr) {
    uint8_t b0 = next_u8(ptr);
    uint8_t b1 = next_u8(ptr);
    uint8_t b2 = next_u8(ptr);
    uint8_t b3 = next_u8(ptr);
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}
#endif

#ifdef PARSER_HELPER_BIG_ENDIAN
uint16_t read_u16_be(char *ptr) {
    uint8_t hi = read_u8(ptr);
    uint8_t lo = read_u8(ptr + 2);
    return (hi << 8) | lo;
}

int16_t  next_i16_be(char **ptr) {
    return (int16_t)next_u16_be(ptr);
}

uint16_t next_u16_be(char **ptr) {
    uint8_t hi = next_u8(ptr);
    uint8_t lo = next_u8(ptr);
    return (hi << 8) | (lo & 0xff);
}

uint32_t next_u24_be(char **ptr) {
    uint8_t hi = next_u8(ptr);
    uint8_t me = next_u8(ptr);
    uint8_t lo = next_u8(ptr);
    return (hi << 16) | (me << 8) | lo;
}

uint32_t next_u32_ne(char **ptr) {
    uint8_t b3 = next_u8(ptr);
    uint8_t b2 = next_u8(ptr);
    uint8_t b1 = next_u8(ptr);
    uint8_t b0 = next_u8(ptr);
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}
#endif

int flat_bin_to_ascii_8(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint8_t data) {
    if (*length + 2 > max_length) return 0;
    buffer[*length] = hex_nibble_high(data);
    buffer[*length + 1] = hex_nibble_low(data);
    *length += 2;
    return 1;
}

int flat_bin_to_ascii_16(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint16_t data) {
    if (!flat_bin_to_ascii_8(buffer, length, max_length, (data & 0xff))) return 0;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, (data >> 8) & 0xff)) return 0;
    return 1;
}

int flat_bin_to_ascii_24(uint8_t *buffer, uint8_t *length, const uint8_t max_length, const uint32_t data) {
    if (!flat_bin_to_ascii_8(buffer, length, max_length, (data & 0xff))) return 0;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, (data >> 8) & 0xff)) return 0;
    if (!flat_bin_to_ascii_8(buffer, length, max_length, (data >> 16) & 0xff)) return 0;
    return 1;
}

int ble_protocol_valid_message(const uint8_t *buffer, const int length) {
    int ccs = 0;
    for (int i = 0; i < length-2; ++i) ccs += buffer[i];
    ccs = ccs % 256;
    uint8_t rcs = read_u8((char *)&buffer[length-2]);
    return ccs == rcs;
}

uint8_t ble_protocol_checksum(const uint8_t *data, const uint8_t length) {
    int cs = 0;
    for (int i = 0; i < length; ++i) cs += data[i];
    return cs % 256;
}

// Helper functions for BleFramePacket

void ble_frame_packet_init_rg_sn(ble_frame_packet_t *packet, uint8_t frame_index, uint32_t rg, uint64_t sn) {
    if (!packet) return;

    packet->metadata.frame_index = frame_index;
    packet->metadata.frame_length = 11;
    packet->frame_data.id = FRAME_ID_RG_SN | FRAME_TYPE_SINGLE;

    // Populate rg (3 bytes, assuming rg is uint32_t but only lower 3 bytes are used)
    // Storing as big-endian in the array (most significant byte first)
    packet->frame_data.data.rg[0] = (rg >> 16) & 0xFF;
    packet->frame_data.data.rg[1] = (rg >> 8) & 0xFF;
    packet->frame_data.data.rg[2] = rg & 0xFF;

    // Populate sn (7 bytes, assuming sn is uint64_t but only lower 7 bytes are used)
    // Storing as big-endian in the array (most significant byte first)
    packet->frame_data.data.sn[0] = (sn >> 48) & 0xFF;
    packet->frame_data.data.sn[1] = (sn >> 40) & 0xFF;
    packet->frame_data.data.sn[2] = (sn >> 32) & 0xFF;
    packet->frame_data.data.sn[3] = (sn >> 24) & 0xFF;
    packet->frame_data.data.sn[4] = (sn >> 16) & 0xFF;
    packet->frame_data.data.sn[5] = (sn >> 8) & 0xFF;
    packet->frame_data.data.sn[6] = sn & 0xFF;
}

void ble_frame_packet_init_component_data(ble_frame_packet_t *packet, uint8_t frame_index, uint16_t component_id, uint32_t distance, uint32_t time) {
    if (!packet) return;

    packet->metadata.frame_index = frame_index;
    packet->metadata.frame_length = 11;
    // packet->frame_data.id = FRAME_ID_COMPONENT_DATA | FRAME_TYPE_SINGLE;
    packet->frame_data.id = component_id | FRAME_TYPE_SINGLE;

    // Convert to network byte order (Big Endian) before storing
    packet->frame_data.data.component_id = ble_protocol_htons(component_id);
    // packet->frame_data.data.distance = htonl(distance);
    // packet->frame_data.data.time = htonl(time);
    packet->frame_data.data.distance = distance;
    packet->frame_data.data.time = time;
}

// Generic encoder: encodes the raw bytes of the (packed and endian-corrected) struct.
int ble_frame_packet_generic_encode_to_hex(const ble_frame_packet_t *packet, uint8_t *output_buffer, uint8_t *current_pos, const uint8_t max_output_length) {
    if (!packet || !output_buffer || !current_pos) return 0;

    memset(output_buffer, 0, max_output_length);

    output_buffer[0] = 0x02;
    strcat((char *)output_buffer, "SET_ADV_MDATA,");
    *current_pos = strlen((char *)output_buffer);

    // Calculate the number of bytes to encode based on the user's formula:
    // sizeof(packet->metadata) (which is 2) + the value stored in packet->metadata.frame_length.
    // packet->metadata.frame_length is expected to be set by init functions to represent
    // the size of the relevant frame_data (i.e., sizeof(id) + length of active union member).
    // For current packet types (13 bytes total): metadata.frame_length should be 11 (1 for id + 10 for data).
    // So, total bytes to encode = 2 (metadata fields) + 11 (value of frame_length field) = 13 bytes.
    const uint8_t packet_binary_size_to_encode = sizeof(packet->metadata) + packet->metadata.frame_length;
    const uint8_t required_hex_len = packet_binary_size_to_encode * 2;

    if ((*current_pos + required_hex_len) > max_output_length) return 0;

    const uint8_t *byte_ptr = (const uint8_t *)packet;
    for (uint8_t i = 0; i < packet_binary_size_to_encode; ++i) {
        if (!flat_bin_to_ascii_8(output_buffer, current_pos, max_output_length, byte_ptr[i])) {
            return 0; // Failed to encode a byte
        }
    }

    output_buffer[*current_pos] = '\n';
    *current_pos += 1;    

    return 1;
}
