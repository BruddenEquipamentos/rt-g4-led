// fminoru on 03/05/2019

#ifndef MB_MASTER_H
#define MB_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "mb_common.h"

extern int (*mb_master_send)(mb_buffer_t *tx);
extern int (*mb_master_wait_recv_or_timeout)();

extern mb_buffer_t mb_master_tx;
extern mb_buffer_t mb_master_rx;

enum mb_master_process {
  MB_MASTER_NO_OP = 0,
  MB_MASTER_OK = 1,
  MB_MASTER_SEND_UNDEFINED = 2,
  MB_MASTER_WAIT_RECV_OR_TIMEOUT_UNDEFINED = 3,
  MB_MASTER_TIMEOUT = 4,
  MB_MASTER_INVALID_CRC = 5,
  MB_MASTER_INVALID_RESPONSE = 6,
  MB_MASTER_NOT_ENABLE = 7,
};

int mb_master_push_rx(u8 *data, u16 length);

int mb_master_read_holding_register(const u8 address, const u16 initial_address,
                                    const u16 num_registers);

int mb_master_write_single_register(const u8 address,
                                    const u16 register_address,
                                    const u16 register_value);

int mb_master_write_multiple_registers(const u8 address,
                                       const u16 initial_address,
                                       const u16 num_registers, u8 byte_count,
                                       const u8 *bytes);

int mb_master_read_device_identification(const u8 address, const u8 mei_type,
                                         const u8 reading_code,
                                         const u8 object_number);

#ifdef __cplusplus
}
#endif

#endif
