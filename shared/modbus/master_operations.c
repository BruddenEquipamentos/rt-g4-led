// fminoru on 06/07/2019
// file should hold the registers to be processed

#include "master_operations.h"
#include <string.h>

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

static size_t u32_to_dec(char *out, size_t cap, uint32_t value) {
    if (cap == 0) return 0;

    char tmp[10];
    size_t len = 0;

    do {
        tmp[len++] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0u && len < sizeof(tmp));

    size_t out_len = len;
    if (out_len > cap - 1u) {
        out_len = cap - 1u;
    }
    for (size_t i = 0; i < out_len; ++i) {
        out[i] = tmp[len - 1u - i];
    }
    out[out_len] = '\0';
    return out_len;
}

static void set_text(char *dst, size_t cap, const char *src) {
    if (cap == 0) return;
    strncpy(dst, src, cap - 1u);
    dst[cap - 1u] = '\0';
}

static void format_vxx(char *dst, size_t cap, uint32_t val) {
    if (cap < 2) return;
    dst[0] = (char)('0' + ((val / 100u) % 10u));
    dst[1] = '.';
    if (cap > 2) dst[2] = (char)('0' + ((val / 10u) % 10u));
    if (cap > 3) dst[3] = (char)('0' + (val % 10u));
    if (cap > 4) dst[4] = '\0';
}

static void format_int_suffix_1(char *dst, size_t cap, uint32_t val, char suffix) {
    if (cap == 0) return;
    size_t len = u32_to_dec(dst, cap, val / 10u);
    if (len + 1u < cap) {
        dst[len++] = '.';
        dst[len++] = (char)('0' + (val % 10u));
        if (suffix != '\0' && len + 1u < cap) {
            dst[len++] = suffix;
        }
        dst[len] = '\0';
    }
}

static void format_uint_suffix(char *dst, size_t cap, uint32_t val, char suffix) {
    if (cap == 0) return;
    size_t len = u32_to_dec(dst, cap, val);
    if (suffix != '\0' && len + 1u < cap) {
        dst[len++] = suffix;
        dst[len] = '\0';
    }
}

static void format_width4(char *dst, size_t cap, uint32_t val) {
    char num[10];
    size_t len = u32_to_dec(num, sizeof(num), val);
    if (cap == 0) return;
    if (len >= 4u) {
        // Keep the right-most digits to fit the display width.
        set_text(dst, cap, &num[len - 4u]);
        return;
    }
    size_t pad = 4u - len;
    size_t i = 0;
    while (i < pad && i + 1u < cap) {
        dst[i++] = ' ';
    }
    size_t j = 0;
    while (j < len && i + 1u < cap) {
        dst[i++] = num[j++];
    }
    dst[i] = '\0';
}

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
        set_text(res_fmtd, RES_FMTD_LENGTH, "----");
        return;
    }

    switch(addr) {
    case 23: format_vxx(res_fmtd, RES_FMTD_LENGTH, op->rval); break;
    case 24: format_vxx(res_fmtd, RES_FMTD_LENGTH, op->rval); break;
    case 25: format_vxx(res_fmtd, RES_FMTD_LENGTH, op->rval); break;
    case 613: format_width4(res_fmtd, RES_FMTD_LENGTH, op->rval); break;

    case 3: format_int_suffix_1(res_fmtd, RES_FMTD_TEMP, op->rval, 'A'); break;
    case 4: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, 'V'); break;
    case 7: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, 'V'); break;
    case 30: format_int_suffix_1(res_fmtd, RES_FMTD_TEMP, op->rval, 'C'); break;

    case 48: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break;
    case 49: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break;
    case 50: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break;
    case 51: format_int_suffix_1(res_fmtd, RES_FMTD_LENGTH, op->rval, 'A'); break;
    case 52: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, 'V'); break;
    case 53: format_int_suffix_1(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break; // todo(fminoru): how to show freq?
    case 54: format_int_suffix_1(res_fmtd, RES_FMTD_TEMP, op->rval, 'C'); break;
    case 60: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break;
    case 61: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break;
    case 70: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break;
    case 1010: format_uint_suffix(res_fmtd, RES_FMTD_LENGTH, op->rval, '\0'); break;
    default: ;
    }
}
