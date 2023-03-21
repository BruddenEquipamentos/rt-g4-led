// fminoru on 06/07/2019
// file should hold the registers to be processed

#include "master_operations.h"

// note(fminoru): import only in one file for binding
// note(fminoru): Register description
//static char p023_desc[] = "Main SW Version";
//static char p024_desc[] = "IO Access. SW Version";
//static char p025_desc[] = "Comm. Acces. SW Version";
//static char p613_desc[] = "Software Revision";

//static char p003_desc[] = "Motor Current";
//static char p004_desc[] = "DC Link Voltage";
//static char p007_desc[] = "Output Voltage";
//static char p030_desc[] = "Module Temperature";

//static char p048_desc[] = "Present Alarm";
//static char p049_desc[] = "Present Fault";
//static char p050_desc[] = "Last Fault";
//static char p051_desc[] = "Last Fault Current";
//static char p052_desc[] = "Last Fault DC Link";
//static char p053_desc[] = "Last Fault Frequency";
//static char p054_desc[] = "Last Fault Temperature";
//static char p060_desc[] = "Seconds Fault";
//static char p061_desc[] = "Third Fault";

//static char p682_desc[] = "Serial/USB Control";
//static char p683_desc[] = "Serial/USB Speed Ref.";
//static char p856_desc[] = "Incline";
//static char p1000_1001_desc[] = "Datetime";
//static char p1010_desc[] = "Collect";

// note(fminoru): Register string value

#define RES_FMTD_LENGTH 5
#define RES_FMTD_TEMP 6
char p023_rfmtd[RES_FMTD_LENGTH] = {0};
char p024_rfmtd[RES_FMTD_LENGTH] = {0};
char p025_rfmtd[RES_FMTD_LENGTH] = {0};
char p613_rfmtd[RES_FMTD_LENGTH] = {0};
char p003_rfmtd[RES_FMTD_TEMP] = {0};
char p004_rfmtd[RES_FMTD_LENGTH] = {0};
char p007_rfmtd[RES_FMTD_LENGTH] = {0};
char p030_rfmtd[RES_FMTD_TEMP] = {0};
char p048_rfmtd[RES_FMTD_LENGTH] = {0};
char p049_rfmtd[RES_FMTD_LENGTH] = {0};
char p050_rfmtd[RES_FMTD_LENGTH] = {0};
char p051_rfmtd[RES_FMTD_LENGTH] = {0};
char p052_rfmtd[RES_FMTD_LENGTH] = {0};
char p053_rfmtd[RES_FMTD_TEMP] = {0};
char p054_rfmtd[RES_FMTD_LENGTH] = {0};
char p060_rfmtd[RES_FMTD_LENGTH] = {0};
char p061_rfmtd[RES_FMTD_LENGTH] = {0};
char p682_rfmtd[RES_FMTD_LENGTH] = {0};
char p683_rfmtd[RES_FMTD_LENGTH] = {0};
char p856_rfmtd[RES_FMTD_LENGTH] = {0};
char p1000_1001_rfmtd[RES_FMTD_LENGTH];
char p1010_rfmtd[RES_FMTD_LENGTH] = {0};

static u8 bytes_4[] = {0};

static void mb_res_fmt(mb_operation_t *op, mb_buffer_t *tx, mb_buffer_t *rx);

mb_operation_t mb_weg_ops[MB_WEG_NUM_OPS] = {
    {.addr = 0x01, .iaddr =  23, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p023_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  24, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p024_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  25, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p025_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr = 613, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p613_rfmtd, .run_once = 1},

    {.addr = 0x01, .iaddr =   3, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p003_rfmtd, .run_once = 1} ,
    {.addr = 0x01, .iaddr =   4, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p004_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =   7, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p007_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  30, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p030_rfmtd, .run_once = 1},

    {.addr = 0x01, .iaddr =  48, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p048_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  49, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p049_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  50, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p050_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  51, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p051_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  52, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p052_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  53, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p053_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  54, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p054_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  60, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p060_rfmtd, .run_once = 1},
    {.addr = 0x01, .iaddr =  61, .nreg = 1, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = p061_rfmtd, .run_once = 1},

    {.addr = 0x01, .raddr = 682, .rval = 0, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = 0},
    {.addr = 0x01, .raddr = 683, .rval = 0, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = 0},
    {.addr = 0x01, .raddr = 856, .rval = 0, .enable = 1, .desc = 0, .res = mb_res_fmt, .rfmtd = 0},

    {.addr = 0x01, .raddr = 1000, .bcount = 4, .bytes = bytes_4, .enable = 1, .desc = 0, .res = mb_res_fmt,
     .rfmtd = 0, .run_once = 0},

    {.addr = 0x01, .iaddr = 1010, .nreg = 1,.enable = 1, .desc = 0, .res = 0, .rfmtd = 0},
};

// note(fminoru): we are adding this as uint16_t instead of pointers to save space
uint16_t mb_ops_display[17] = {23, 24, 25, 613, 3, 4, 7, 30, 48, 49, 50, 51, 52, 53, 54, 60, 61};

uint32_t mb_operations_display_length = 0;

static void mb_res_fmt(mb_operation_t *op, mb_buffer_t *tx, mb_buffer_t *rx) {

    u16 addr = op->iaddr != 0 ? op->iaddr : op->raddr;

    char *res_fmtd = op->rfmtd;

    if (!res_fmtd) {
        return;
    }

    if (op->rval == 0) {
        snprintf(res_fmtd, RES_FMTD_LENGTH, "----");
        return;
    }

    switch(addr) {
    case 23: snprintf(res_fmtd, RES_FMTD_LENGTH, "%1d.%02d", op->rval/100, op->rval%100); break;
    case 24: snprintf(res_fmtd, RES_FMTD_LENGTH, "%1d.%02d", op->rval/100, op->rval%100); break;
    case 25: snprintf(res_fmtd, RES_FMTD_LENGTH, "%1d.%02d", op->rval/100, op->rval%100); break;
    case 613:snprintf(res_fmtd, RES_FMTD_LENGTH, "%4d", op->rval); break;

    case 3: snprintf(res_fmtd, RES_FMTD_TEMP, "%2d.%1dA", op->rval/10, op->rval%10); break;
    case 4: snprintf(res_fmtd, RES_FMTD_LENGTH, "%3dV", op->rval); break;
    case 7: snprintf(res_fmtd, RES_FMTD_LENGTH, "%3dV", op->rval); break;
    case 30:snprintf(res_fmtd, RES_FMTD_TEMP, "%2d.%1dC", op->rval/10, op->rval%10); break;

    case 48:snprintf(res_fmtd, RES_FMTD_LENGTH, "%d", op->rval); break;
    case 49:snprintf(res_fmtd, RES_FMTD_LENGTH, "%d", op->rval); break;
    case 50:snprintf(res_fmtd, RES_FMTD_LENGTH, "%d", op->rval); break;
    case 51:snprintf(res_fmtd                                   , RES_FMTD_LENGTH, "%d.%1dA", op->rval/10, op->rval%10); break;
    case 52:snprintf(res_fmtd, RES_FMTD_LENGTH, "%dV", op->rval); break;
    case 53:snprintf(res_fmtd, RES_FMTD_LENGTH, "%d.%1d", op->rval/10, op->rval%10); break; // todo(fminoru): how to show freq?
    case 54:snprintf(res_fmtd, RES_FMTD_TEMP, "%d.%1dC", op->rval/10, op->rval%10); break;
    case 60:snprintf(res_fmtd, RES_FMTD_LENGTH, "%d", op->rval); break;
    case 61:snprintf(res_fmtd, RES_FMTD_LENGTH, "%d", op->rval); break;
    case 70:snprintf(res_fmtd, RES_FMTD_LENGTH, "%d", op->rval); break;
    case 1010: snprintf(res_fmtd, RES_FMTD_LENGTH, "%d", op->rval); break;
    default: ;
    }
}
