#include "dm.h"
void (*_dot_matrix_render)(const uint16_t dot_matrix[5]) = 0;

uint16_t dot_matrix_buffer[5];

void dot_matrix_render(void) {
  if (_dot_matrix_render != 0) _dot_matrix_render(dot_matrix_buffer);
}


void dot_matrix_clear_buffer(void) {
  for (int i = 0; i < 5; ++i) dot_matrix_buffer[i] = 0x0000;
}

void dot_matrix_init() { dot_matrix_clear_buffer(); }

static void dot_matrix_render_to_buffer(const digit_t *digit, int x) {
  for (int row = 0; row < 5; ++row) {
    for (int k = digit->cols - 1, dx = 0; k >= 0; --k, ++dx) {
      if (x + dx >= 16 || x + dx < 0) continue;
      if ((digit->data[row] >> k) & 1) {
        dot_matrix_buffer[row] =
            dot_matrix_buffer[row] | (1 << (15 - (x + dx)));
      } else {
        dot_matrix_buffer[row] &= ~(0 << (15 - (x + dx)));
      }
    }
  }
}

void dot_matrix_render_column_line(int col) {
  if (col > 15) return;
  for (int row = 0; row < 5; ++row) {
    dot_matrix_buffer[row] |= (1<<(15-col));
  }
}

void dot_matrix_print_number(int decimal) {
  dot_matrix_clear_buffer();
  dot_matrix_render_to_buffer(dm_number(decimal), 6);
}

void dot_matrix_render_text(char *text, int frame) {
  int start_x = 16;
  dot_matrix_clear_buffer();
  while (*text) {
    const digit_t *dm = dm_digit(*text);
    if (dm == 0) break;
    dot_matrix_render_to_buffer(dm, start_x - frame);
    start_x += (dm->cols + 1);
    text++;
  }
}

void dot_matrix_print_program_number(const int number) {
  dot_matrix_clear_buffer();
  if (number < 10) {
    dot_matrix_render_to_buffer(&dm_P, 5);
    dot_matrix_render_to_buffer(dm_number(number), 9);
  } else {
    dot_matrix_render_to_buffer(&dm_P, 4);
    dot_matrix_render_to_buffer(dm_number(number / 10), 8);
    dot_matrix_render_to_buffer(dm_number(number % 10), 12);
  }

  dot_matrix_render();
}
