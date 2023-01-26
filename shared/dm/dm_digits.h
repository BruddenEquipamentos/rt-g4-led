#ifndef DM_DIGITS_H
#define DM_DIGITS_H

#include <stdint.h>

// note: this is temporary.
// it is currently shared between desktop_panel and led_panel

typedef struct digit {
    uint8_t cols;
    uint8_t data[5];
} digit_t;

typedef struct digit_map {
    const char ch;
    const digit_t *dm;
} digit_map_t;

extern const digit_t dm_percentage;
extern const digit_t dm_hyphen;
extern const digit_t dm_slash;
extern const digit_t dm_heart;
extern const digit_t dm_0;
extern const digit_t dm_1;
extern const digit_t dm_2;
extern const digit_t dm_3;
extern const digit_t dm_4;
extern const digit_t dm_5;
extern const digit_t dm_6;
extern const digit_t dm_7;
extern const digit_t dm_8;
extern const digit_t dm_9;
extern const digit_t dm_A;
extern const digit_t dm_C;
extern const digit_t dm_D;
extern const digit_t dm_E;
extern const digit_t dm_F;
extern const digit_t dm_G;
extern const digit_t dm_H;
extern const digit_t dm_I;
extern const digit_t dm_L;
extern const digit_t dm_M;
extern const digit_t dm_N;
extern const digit_t dm_O;
extern const digit_t dm_P;
extern const digit_t dm_R;
extern const digit_t dm_S;
extern const digit_t dm_T;
extern const digit_t dm_U;
extern const digit_t dm_W;
extern const digit_t dm_X;
extern const digit_t dm_x;

extern digit_map_t dm_map[];

const digit_t *dm_number(const int number);
const digit_t *dm_digit(char ch);

int dm_compute_length(char *text);

#endif
