// fminoru on 06/07/2019
// file should hold the registers to be processed

#ifndef MASTER_OPERATIONS_H
#define MASTER_OPERATIONS_H

#include "mb_master.h"
#include "mb_operation.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MB_WEG_NUM_OPS 22
    extern mb_operation_t mb_weg_ops[];

    extern uint16_t mb_ops_display[17];

    extern uint32_t mb_operations_display_length;


#ifdef __cplusplus
}
#endif


#endif
