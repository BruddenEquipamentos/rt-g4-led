#ifndef DIGITS_H
#define DIGITS_H

#include <stdint.h>

typedef enum {
	DIGIT_0 = 0,
	DIGIT_1,
	DIGIT_2,
	DIGIT_3,
	DIGIT_4,
	DIGIT_5,
	DIGIT_6,
	DIGIT_7,
	DIGIT_8,
	DIGIT_9,
	DIGIT_P,
	DIGIT_UNDERLINE,
	DIGIT_MIDDLE,
	DIGIT_d,
	DIGIT_f,
	DIGIT_U,
	DIGIT_E,
	DIGIT_r,
	DIGIT_t,
	DIGIT_b,
	DIGIT_L,
	DIGIT_A,
	DIGIT_o,
	DIGIT_EMPTY,
	DIGIT_TOTALS
} sdigit_t;

extern const uint8_t segment_digit[DIGIT_TOTALS];

#endif
