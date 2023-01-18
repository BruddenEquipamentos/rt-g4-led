#ifndef D_DOT_MATRIX_H
#define D_DOT_MATRIX_H

#include <stdint.h>

#include "d_dot_matrix_digits.h"

void dot_matrix_render(void);
void dot_matrix_clear_buffer(void);

void dot_matrix_render_column_line(int col);


void dot_matrix_init(void);
void dot_matrix_print_buffer(void);

void dot_matrix_render_text(char *text, int frame);
void dot_matrix_print_program_number(const int number);

void dot_matrix_print_number(int decimal);

extern void (*_dot_matrix_render)(const uint16_t dot_matrix[5]);

extern uint16_t dot_matrix_buffer[5];

const digit_t *dm_number(const int number);

#endif
