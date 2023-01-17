#ifndef DOT_MATRIX_H
#define DOT_MATRIX_H

#include "d_digits.h"

void dot_matrix_render_program(uint8_t *buffer, int program_number);
void dot_matrix_render_load_level(uint8_t *buffer, int load_level);
void dot_matrix_render_age(uint8_t *buffer);
void dot_matrix_clear_buffer(uint8_t *buffer);
void dot_matrix_render_number(uint8_t *buffer, int decimal);
void dot_matrix_render_serie(uint8_t *buffer, int current_serie, int total_series);

void dot_matrix_render_horizontal_line(uint8_t *buffer, const uint8_t y);
void dot_matrix_render_horizontal_line_dots(uint8_t *buffer, const uint8_t y, const int num_dots);

void dot_matrix_render_column_line(uint8_t *buffer, const uint8_t x);

void dot_matrix_render_dot(uint8_t *buffer, const uint8_t x, const uint8_t y);

void dot_matrix_render_paused(uint8_t *buffer, int frame);
void dot_matrix_render_report(uint8_t *buffer, int frame);
void dot_matrix_render_finished(uint8_t *buffer, int frame);

void dot_matrix_render_text(uint8_t *buffer, char *text, int frame);

#endif
