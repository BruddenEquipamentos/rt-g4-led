// fminoru on 06/05/2019

#ifndef MB_COMMON_H
#define MB_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <time.h>

    typedef uint8_t  u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef int16_t s16;
    typedef int32_t s32;

    enum Function {
        FREAD_HOLDING_REGISTER = 0x03,
        FWRITE_SINGLE_REGISTER = 0x06,
        FWRITE_MULTIPLE_REGISTERS = 0x10,
        FREAD_DEVICE_IDENTIFICATION = 0x2B,
    };

    typedef struct mb_buffer {
        u8 data[256];
        u16 length;
    } mb_buffer_t;

    uint16_t mb_common_crc16(u8 *data, u16 length);
    void mb_common_buffer_crc16(mb_buffer_t *buffer);

    enum mb_exception_code {
        MB_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
        MB_EXCEPTION_ILLEGAL_DATA_ADDRESS = 0x02,
        MB_EXCEPTION_ILLEGAL_DATA_VALUE = 0x03,
    };

/* note(fminoru): Need to Handle all exception cases?
   http://www.simplymodbus.ca/exceptions.htm
   Exception Codes
   0x01 - Illegal function (ok)
   0x02 - Illegal data address (ok)
   0x03 - Illegal data value (ok)
   0x04 - Slave device failure
   0x05 - Acknowledge
   0x06 - Slave device busy
   0x07 - Negative Acknowledge
   0x08 - Memory Parity Error
   0x0A - Gateway path unavailable
   0x0B - Gatewat target deviec failed to respond
*/

#ifdef __cplusplus
}
#endif

#endif
