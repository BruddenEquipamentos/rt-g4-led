#include "dm.h"
void (*_dm_render)(const uint16_t dot_matrix[5]) = 0;

uint16_t dm_buffer[5];
uint32_t dm_tick = 0;
int dm_frame = 0;

void dm_render(void) {
  if (_dm_render != 0) _dm_render(dm_buffer);
}

void dm_clear_buffer(void) {
  for (int i = 0; i < 5; ++i) dm_buffer[i] = 0x0000;
}

void dm_init() { dm_clear_buffer(); }

static void dm_render_to_buffer(const digit_t *digit, int x) {
  for (int row = 0; row < 5; ++row) {
    for (int k = digit->cols - 1, dx = 0; k >= 0; --k, ++dx) {
      if (x + dx >= 16 || x + dx < 0) continue;
      if ((digit->data[row] >> k) & 1) {
        dm_buffer[row] =
            dm_buffer[row] | (1 << (15 - (x + dx)));
      } else {
        dm_buffer[row] &= ~(0 << (15 - (x + dx)));
      }
    }
  }
}

void dm_render_column_line(int col) {
  if (col > 15) return;
  for (int row = 0; row < 5; ++row) {
    dm_buffer[row] |= (1<<(15-col));
  }
}

void dm_render_number(int decimal) {
  dm_clear_buffer();
  dm_render_to_buffer(dm_number(decimal), 6);
}

void dm_render_text(char *text, int frame) {
  int start_x = 16;
  dm_clear_buffer();
  while (*text) {
    const digit_t *dm = dm_digit(*text);
    if (dm == 0) break;
    dm_render_to_buffer(dm, start_x - frame);
    start_x += (dm->cols + 1);
    text++;
  }
}

void dm_print_program_number(const int number) {
  dm_clear_buffer();
  if (number < 10) {
    dm_render_to_buffer(&dm_P, 5);
    dm_render_to_buffer(dm_number(number), 9);
  } else {
    dm_render_to_buffer(&dm_P, 4);
    dm_render_to_buffer(dm_number(number / 10), 8);
    dm_render_to_buffer(dm_number(number % 10), 12);
  }

  dm_render();
}
