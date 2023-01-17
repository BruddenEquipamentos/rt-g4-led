#ifndef DOTMATRIX_H
#define DOTMATRIX_H

#include <stdint.h>

#include "../shared/d_digits.h"

extern int dotmatrix_offset;

void dotmatrix_clear_buffer();
void dotmatrix_init();
void dotmatrix_render(const digit_t *digit, int x);

void dotmatrix_print_buffer();
void dotmatrix_render_pause(int frame);

void dotmatrix_render_text(char *text, int frame);
void dotmatrix_print_program_number(const int number);

void dotmatrix_print_number(int decimal);

const digit_t *dm_number(const int number);

#endif
