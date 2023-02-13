// fminoru on 11/06/2019

#include "mb_operation.h"

#include <string.h>

mb_operation_t *mb_operations = 0;
uint16_t mb_operations_length = 0;

//#define MB_DEBUG_PRINT

#ifdef MB_DEBUG_PRINT
static char status_desc[][20] = {
    " ",
    "[OK]",
    " ",
    " ",
    "[TIMEOUT]",
    "[CRC NOT VALID]"
};
#endif

static void dprint_desc(mb_operation_t *op)
{
#ifdef MB_DEBUG_PRINT
    if (op->desc) {
        printf("[TX] [RD] [%-50s]", op->desc);
    } else {
        static char desc[50];
        snprintf(desc, 50, "iaddr:%04d nreg:%04d raddr:%04d bcount:%04d", op->iaddr, op->nreg, op->raddr, op->bcount);
        printf("[TX] [RD] [%-50s]", desc);
    }
#endif
}

static void dprint_status(char status_code)
{
#ifdef MB_DEBUG_PRINT
    printf("%s", status_desc[status_code]);
#endif
}

static void dprint_value(mb_operation_t *op)
{
#ifdef MB_DEBUG_PRINT
    if (!op->rfmtd) return;
    printf(" [%s]", op->rfmtd);
#endif
}

static void dprint_newline()
{
#ifdef MB_DEBUG_PRINT
    printf("\n");
#endif
}

static void dprint_exception(u8 exception) {
#ifdef MB_DEBUG_PRINT
    switch (exception) {
    case MB_EXCEPTION_ILLEGAL_FUNCTION: printf("[Illegal Function]"); break;
    case MB_EXCEPTION_ILLEGAL_DATA_ADDRESS: printf("[Illegal Data Address]"); break;
    case MB_EXCEPTION_ILLEGAL_DATA_VALUE: printf("[Illegal Data Value]"); break;
    default: printf("[Exception not implemented]");
    }
#endif
}

mb_operation_t *mb_operation_fetch_next_op() {
    static size_t index = 0;
    mb_operation_t *op;
    size_t current_index = index;
    do {
        op = &mb_operations[index++];
        if (index >= mb_operations_length) index = 0;
    } while (!op->enable && index != current_index);

    return op;
}

mb_operation_t *mb_operation_get(mb_operation_t *ops, uint16_t length, uint16_t addr) {
    if (!ops) return 0;
    for (int i = 0; i < length; ++i) {
        mb_operation_t *op = (mb_operation_t *)&ops[i];
        u16 raddr = op->iaddr ? op->iaddr : op->raddr;
        if (raddr == addr) {
            return op;
        }
    }
    return 0;
}

extern void vDrvDebugSendStr(const char *str);
#define VDEBUG(str) vDrvDebugSendStr(str);

enum mb_master_process mb_operation_process(mb_operation_t *op) {
    enum mb_master_process status = MB_MASTER_OK;

    if (!op) return MB_MASTER_NO_OP;

    if (!op->enable) return MB_MASTER_NOT_ENABLE;

    /* static char str[20]; */
    /* snprintf(str, 20, "op: %d res: ", op->iaddr ? op->iaddr : op->raddr); */
    /* VDEBUG(str); */

    if (op->iaddr) {
        dprint_desc(op);
    	status = mb_master_read_holding_register(op->addr, op->iaddr, op->nreg);
        dprint_status(status);

        if (status == MB_MASTER_OK && op->nreg == 1) {
            op->rval = (u16) ((mb_master_rx.data[3] << 8) + mb_master_rx.data[4]);
        }

        if (status == MB_MASTER_OK && op->nreg > 1) {
            // does it have op bytes?
            if (op->bytes) {
                op->bcount = mb_master_rx.data[2];
                memcpy(op->bytes, &mb_master_rx.data[3], op->bcount);
            }
        }

    } else if (op->raddr) {
        if (op->bcount > 0 && op->bytes != 0) {
            dprint_desc(op);
            status = mb_master_write_multiple_registers(op->addr, op->raddr, op->bcount/2, op->bcount, op->bytes);
            dprint_status(status);
        } else {
            dprint_desc(op);
            status = mb_master_write_single_register(op->addr, op->raddr, op->rval);
            dprint_status(status);
        }
    } else {
        // illegal function code
    }

    if (status == MB_MASTER_OK) {
        if (op->run_once) {
            op->enable = 0;
        }
        if (op->res) {
            op->res(op, &mb_master_tx, &mb_master_rx);
            dprint_value(op);
        }
    } else if (status == MB_MASTER_INVALID_RESPONSE) {
        dprint_exception(mb_master_rx.data[2]);
        // todo(fminoru): handle invalid response
    }
    dprint_newline();

    /* snprintf(str, 20, "%d length: %d\n", status, mb_master_tx.length); */
    /* VDEBUG(str); */

    /* switch (status) { */
    /* case MB_MASTER_NO_OP:            VDEBUG("MB_MASTER_NO_OP\n"); break; */
    /* case MB_MASTER_OK:               VDEBUG("MB_MASTER_OK\n"); break; */
    /* case MB_MASTER_TIMEOUT:          VDEBUG("MB_MASTER_TIMEOUT\n"); break; */
    /* case MB_MASTER_INVALID_CRC:      VDEBUG("MB_MASTER_INVALID_CRC\n"); break; */
    /* case MB_MASTER_INVALID_RESPONSE: VDEBUG("MB_MASTER_INVALID_RESPONSE\n"); break; */
    /* case MB_MASTER_NOT_ENABLE:       VDEBUG("MB_MASTER_NO_OP\n"); break; */
    /* default: ; */
    /* } */

    return status;
}

int mb_operation_reenable_run_once(void) {
    if (!mb_operations) return 0;
    for (int i = 0; i < mb_operations_length; ++i) {
        mb_operation_t *op = (mb_operation_t *)&mb_operations[i];
        if (op->run_once && !op->enable) op->enable = 1;
    }
    return 1;
}
