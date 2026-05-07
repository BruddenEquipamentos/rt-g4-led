#ifndef DM_H
#define DM_H

#include <stdint.h>

#include "dm_digits.h"

typedef enum dm_text_position {
  DM_TEXT_POSITION_LEFT,
  DM_TEXT_POSITION_CENTER,
  DM_TEXT_POSITION_RIGHT,
} dm_text_position_t;

void dm_reset_frame(void);
void dm_reset_frame_messages(void); ///

void dm_update(void);
void dm_clear_buffer(void);

void dm_render_column_line(int col);

void dm_init(void);
void dm_print_buffer(void);

void dm_render_number(int decimal);
void dm_render_text(const char *text, int frame);
int dm_render_text_animated(const char *text, const uint32_t ms_per_frame,
                             const uint32_t ms);

extern void (*_dm_update)(const uint16_t dot_matrix[5]);

extern uint16_t dm_buffer[5];

const digit_t *dm_number(const int number);

void dm_render_line(int line);
void dm_render_line_column(int line, int column);
void dm_render_column_with_height(int col, int height, uint32_t current_ms, uint32_t blink_ms);

void dm_render_lines(uint32_t ms_per_frame, uint32_t ms);

#endif
