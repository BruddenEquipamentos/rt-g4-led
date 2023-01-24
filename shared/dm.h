#ifndef DM_H
#define DM_H

#include <stdint.h>

#include "dm_digits.h"

extern int dm_frame;
extern uint32_t dm_tick;

void dm_render(void);
void dm_clear_buffer(void);

void dm_render_column_line(int col);


void dm_init(void);
void dm_print_buffer(void);

void dm_render_text(char *text, int frame);
void dm_print_program_number(const int number);

void dm_render_number(int decimal);

extern void (*_dm_render)(const uint16_t dot_matrix[5]);

extern uint16_t dm_buffer[5];

const digit_t *dm_number(const int number);

#endif
