// fminoru on 11/06/2019

#ifndef MB_OPERATION_H
#define MB_OPERATION_H

#include "mb_master.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct mb_operation mb_operation_t;

    extern mb_operation_t *mb_operations;
    extern uint16_t mb_operations_length;

    typedef struct mb_operation {
        u8 enable;

        // device address
        u8 addr;

        // Read Reg (iaddr, nreg)
        // Write Mul (raddr, bcount, bytes)
        u8  bcount;
        u8 *bytes;
        u16 iaddr;
        u16 nreg;

        // Write Single (raddr, rval)
        u16 raddr;
        u16 rval;

        void (*res)(struct mb_operation *op, mb_buffer_t *tx, mb_buffer_t *rx);

        struct mb_operation *next;

        char *desc;

        char run_once;

        char *rfmtd;

    } mb_operation_t;

    // note(fminoru): for this thing to work, we must first define
    // and array and it's size.

    enum mb_master_process mb_operation_process(mb_operation_t *op);

    mb_operation_t *mb_operation_get(mb_operation_t *ops, uint16_t length, uint16_t addr);

    mb_operation_t *mb_operation_fetch_next_op();

    //mb_operation_t* mb_operation_process(mb_operation_t *op);

    int mb_operation_reenable_run_once(void);


#ifdef __cplusplus
}
#endif


#endif
