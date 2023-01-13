#include "dotmatrix.h"

int dotmatrix_offset = 0;

static uint16_t dotmatrix_buffer[5];

void dotmatrix_clear_buffer() {
  for (int i = 0; i < 5; ++i) dotmatrix_buffer[i] = 0x0000;
}

void dotmatrix_init() { dotmatrix_clear_buffer(); }

#include <stdio.h>

void dotmatrix_render(const digit_t *digit, int x) {
  for (int row = 0; row < 5; ++row) {
    for (int k = digit->cols - 1, dx = 0; k >= 0; --k, ++dx) {
      if (x + dx >= 16 || x + dx < 0) continue;
      if ((digit->data[row] >> k) & 1) {
        dotmatrix_buffer[row] = dotmatrix_buffer[row] | (1 << (15 - (x + dx)));
      } else {
        dotmatrix_buffer[row] &= ~(0 << (15 - (x + dx)));
      }
    }
  }
}

void dotmatrix_print_buffer() {
  for (int row = 0; row < 5; row++) {
    for (int i = 0; i < dotmatrix_offset; ++i) fprintf(stdout, " ");
    for (int col = 15; col >= 0; --col) {
      if ((dotmatrix_buffer[row] >> col) & 1)
        fprintf(stdout, "▗");
      else
        fprintf(stdout, " ");
    }
    fprintf(stdout, "\n");
  }
}

void dotmatrix_render_pause(int frame) {
  dotmatrix_clear_buffer();
  dotmatrix_render(&dm_P, 16 - frame);
  dotmatrix_render(&dm_A, 20 - frame);
  dotmatrix_render(&dm_U, 24 - frame);
  dotmatrix_render(&dm_S, 28 - frame);
  dotmatrix_render(&dm_E, 32 - frame);
  dotmatrix_print_buffer();
}

void dotmatrix_print_number(int decimal) {
  dotmatrix_clear_buffer();
  dotmatrix_render(dm_number(decimal), 6);
}

void dotmatrix_render_text(char *text, int frame) {
  int start_x = 16;
  dotmatrix_clear_buffer();
  while (*text) {
    const digit_t *dm = dm_digit(*text);
    if (dm == 0) break;
    dotmatrix_render(dm, start_x - frame);
    start_x += (dm->cols + 1);
    text++;
  }
  dotmatrix_print_buffer();
}

void dotmatrix_print_program_number(const int number) {
  dotmatrix_clear_buffer();
  if (number < 10) {
    dotmatrix_render(&dm_P, 5);
    dotmatrix_render(dm_number(number), 9);
  } else {
    dotmatrix_render(&dm_P, 4);
    dotmatrix_render(dm_number(number / 10), 8);
    dotmatrix_render(dm_number(number % 10), 12);
  }
  dotmatrix_print_buffer();
}
