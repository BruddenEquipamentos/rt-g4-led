// fminoru on 03/05/2019

#include "mb_master.h"

#include "mb_common.h"

int (*mb_master_send)(mb_buffer_t *tx) = NULL;

int (*mb_master_wait_recv_or_timeout)() = NULL;

mb_buffer_t mb_master_tx = {.data = {0}, .length = 0};
mb_buffer_t mb_master_rx = {.data = {0}, .length = 0};

static int mb_master_validate_crc(u8 *data, u16 length) {
  u16 crc;
  u16 data_crc;
  /*
    note(fminoru): see WEG CFW 300 doc.
    FREAD_HOLDING_REGISTER: 2 bytes + byte_count bytes +  2 bytes crc
    FWRITE_SINGLE_REGISTER: 6 bytes + 2 bytes crc
    FWRITE_MULTIPLE_REGISTERS: 6 bytes + 2 bytes crc
    FREAD_DEVICE_IDENTIFICATION: 5 bytes + 2 bytes crc
  */
  if (length <= 0) return 0;

  switch (data[1]) {
    case FREAD_HOLDING_REGISTER: {
      u16 byte_count = data[2];
      crc = mb_common_crc16(data, 3 + byte_count);
      data_crc = (data[length - 1] << 8) + (data[length - 2]);
      break;
    }
    case FWRITE_SINGLE_REGISTER: {
      crc = mb_common_crc16(data, 6);
      data_crc = (data[7] << 8) + (data[6]);
      break;
    }
    case FWRITE_MULTIPLE_REGISTERS: {
      crc = mb_common_crc16(data, 6);
      data_crc = (data[7] << 8) + (data[6]);
      break;
    }
    case FREAD_DEVICE_IDENTIFICATION: {
      // todo(fminoru): Implement]
      crc = mb_common_crc16(data, 5);
      data_crc = (data[6] << 8) + (data[5]);
      return 1;
      break;
    }
    default: {
      return -1;
      break;
    }
  }
  return crc == data_crc;
}

static int mb_master_validate_function_response(u8 *data, u16 length) {
  if (data[1] >> 7) {
    return 0;
  }

  if (length > 0) {
    if (mb_master_tx.data[0] != mb_master_rx.data[0] ||
        mb_master_tx.data[1] != mb_master_rx.data[1]) {
      return 0;
    }
  }

  return 1;
}

static void mb_buffer_clear(struct mb_buffer *buffer) {
  for (int i = 0; i < 256; ++i) buffer->data[i] = 0x00;
  buffer->length = 0;
}

static int mb_master_process_tx_rx() {
  if (!mb_master_send) return MB_MASTER_SEND_UNDEFINED;
  if (!mb_master_wait_recv_or_timeout)
    return MB_MASTER_WAIT_RECV_OR_TIMEOUT_UNDEFINED;

  mb_buffer_clear(&mb_master_rx);

  mb_master_send(&mb_master_tx);

  if (mb_master_wait_recv_or_timeout()) {
    // todo(fminoru): refactor mb_master_wait_recv_or_timeout() name to be
    // timeout or not specific
    return MB_MASTER_TIMEOUT;
  }

  // rx should be feed using the function mb_master_push_rx
  if (!mb_master_validate_crc(mb_master_rx.data, mb_master_rx.length)) {
    return MB_MASTER_INVALID_CRC;
  }

  // check if we didnt get an error
  if (!mb_master_validate_function_response(mb_master_rx.data,
                                            mb_master_rx.length)) {
    return MB_MASTER_INVALID_RESPONSE;
  }
  return MB_MASTER_OK;
}

int mb_master_read_holding_register(const u8 address, const u16 initial_address,
                                    const u16 num_registers) {
  mb_master_tx.data[0] = address;
  mb_master_tx.data[1] = FREAD_HOLDING_REGISTER;
  mb_master_tx.data[2] = (initial_address >> 8) & 0xff;
  mb_master_tx.data[3] = (initial_address & 0xff);
  mb_master_tx.data[4] = (num_registers >> 8) & 0xff;
  mb_master_tx.data[5] = (num_registers & 0xff);
  mb_master_tx.length = 6;
  mb_common_buffer_crc16(&mb_master_tx);
  return mb_master_process_tx_rx();
}

int mb_master_write_single_register(const u8 address,
                                    const u16 register_address,
                                    const u16 register_value) {
  mb_master_tx.data[0] = address;
  mb_master_tx.data[1] = FWRITE_SINGLE_REGISTER;
  mb_master_tx.data[2] = (register_address >> 8) & 0xff;
  mb_master_tx.data[3] = (register_address & 0xff);
  mb_master_tx.data[4] = (register_value >> 8) & 0xff;
  mb_master_tx.data[5] = (register_value & 0xff);
  mb_master_tx.length = 6;
  mb_common_buffer_crc16(&mb_master_tx);
  return mb_master_process_tx_rx();
}

int mb_master_write_multiple_registers(const u8 address,
                                       const u16 initial_address,
                                       const u16 num_registers, u8 byte_count,
                                       const u8 *bytes) {
  mb_master_tx.data[0] = address;
  mb_master_tx.data[1] = FWRITE_MULTIPLE_REGISTERS;
  mb_master_tx.data[2] = (initial_address >> 8) & 0xff;
  mb_master_tx.data[3] = (initial_address & 0xff);
  mb_master_tx.data[4] = (num_registers >> 8) & 0xff;
  mb_master_tx.data[5] = (num_registers & 0xff);
  mb_master_tx.data[6] = byte_count;
  mb_master_tx.length = 7;
  while (byte_count) {
    mb_master_tx.data[mb_master_tx.length++] =
        bytes[mb_master_tx.data[6] - byte_count];
    byte_count--;
  }
  mb_common_buffer_crc16(&mb_master_tx);
  return mb_master_process_tx_rx();
}

int mb_master_read_device_identification(const u8 address, const u8 mei_type,
                                         const u8 reading_code,
                                         const u8 object_number) {
  mb_master_tx.data[0] = address;
  mb_master_tx.data[1] = FREAD_DEVICE_IDENTIFICATION;
  mb_master_tx.data[2] = mei_type;
  mb_master_tx.data[3] = reading_code;
  mb_master_tx.data[4] = object_number;
  mb_master_tx.length = 5;
  mb_common_buffer_crc16(&mb_master_tx);
  return mb_master_process_tx_rx();
}

int mb_master_push_rx(u8 *data, u16 length) {
  if (mb_master_rx.length + length > 256) {
    return 0;
  }

  while (length) {
    mb_master_rx.data[mb_master_rx.length++] = *data++;
    length--;
  }

  return 1;
}
