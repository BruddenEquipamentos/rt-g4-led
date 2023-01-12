#include "dot_matrix.h"

static const uint8_t column_address[16] = {
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};

static const uint8_t row_value[5] = {
    0x01, 0x02, 0x04, 0x08, 0x10};

static void dot_matrix_render_to_buffer(uint8_t *buffer, const int start_column, const digit_t *dm) {
  for (uint8_t row = 0; row < 5; row++) {
    for (int column = 0; column < dm->cols; column++) {
      int dx = start_column + column;
      if (dx < 0 || dx >= 16) continue;
      if ((dm->data[row] >> (dm->cols - 1 - column)) & 1) {
        buffer[column_address[dx]] |= row_value[row];
      } else {
        buffer[column_address[dx]] &= ~row_value[row];
      }
    }
  }
}

void dot_matrix_render_horizontal_line(uint8_t *buffer, const uint8_t y) {
  for (uint8_t column = 0; column < 16; column++) {
    buffer[column_address[column]] |= row_value[y];
  }
}

void dot_matrix_render_horizontal_line_dots(uint8_t *buffer, const uint8_t y, const int num_dots) {
  for (uint8_t column = 0; column < num_dots; column++) {
    buffer[column_address[column]] |= row_value[y];
  }
}

void dot_matrix_render_column_line(uint8_t *buffer, const uint8_t x) {
  for (uint8_t row = 0; row < 5; ++row) {
    buffer[column_address[x]] |= row_value[row];
  }
}

void dot_matrix_clear_horizontal_line(uint8_t *buffer, const uint8_t y) {
  for (uint8_t column = 0; column < 16; column++) {
    buffer[column_address[column]] &= ~row_value[y];
  }
}

void dot_matrix_render_dot(uint8_t *buffer, const uint8_t x, const uint8_t y) {
  buffer[column_address[x]] |= row_value[y];
}

void dot_matrix_clear_buffer(uint8_t *buffer) {
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t column = 0; column < 16; column++) {
      buffer[column_address[column]] = 0x0000;
    }
  }
}

void dot_matrix_render_serie(uint8_t *buffer, int current_serie, int total_series) {
  dot_matrix_clear_buffer(buffer);
  dot_matrix_render_to_buffer(buffer, 3, dm_number(current_serie));
  dot_matrix_render_to_buffer(buffer, 7, &dm_hyphen);
  dot_matrix_render_to_buffer(buffer, 11, dm_number(total_series));
}

void dot_matrix_render_load_level(uint8_t *buffer, int load_level) {
  (void)buffer;
  (void)load_level;
  // dot_matrix_clear_buffer(buffer);
  // if (load_level < 10) {
  //   dot_matrix_render_to_buffer(buffer, 1, &dm_L2c);
  //   dot_matrix_render_to_buffer(buffer, 4, &dm_v);
  //   dot_matrix_render_to_buffer(buffer, 12, get_digit(load_level));
  // } else {
  //   int d = (load_level / 10) % 10;
  //   int r = (load_level % 10);
  //   dot_matrix_render_to_buffer(buffer, 1, &dm_L2c);
  //   dot_matrix_render_to_buffer(buffer, 4, &dm_v);

  //   if (d == 1)
  //     dot_matrix_render_to_buffer(buffer, 9, get_digit(d));
  //   else
  //     dot_matrix_render_to_buffer(buffer, 8, get_digit(d));
  //   dot_matrix_render_to_buffer(buffer, 12, get_digit(r));
  // }
}

void dot_matrix_render_program(uint8_t *buffer, int program_number) {
  if (program_number < 10) {
    dot_matrix_render_to_buffer(buffer, 4, &dm_P);
    dot_matrix_render_to_buffer(buffer, 9, dm_number(program_number));
  } else {
    int d = (program_number / 10) % 10;
    int r = (program_number % 10);
    dot_matrix_render_to_buffer(buffer, 2, &dm_P);
    dot_matrix_render_to_buffer(buffer, 7, dm_number(d));
    dot_matrix_render_to_buffer(buffer, 11, dm_number(r));
  }
}

void dot_matrix_render_age(uint8_t *buffer) {
  dot_matrix_clear_buffer(buffer);
  dot_matrix_render_to_buffer(buffer, 2, &dm_A);
  dot_matrix_render_to_buffer(buffer, 6, &dm_G);
  dot_matrix_render_to_buffer(buffer, 10, &dm_E);
}

void dot_matrix_render_number(uint8_t *buffer, int decimal) {
  dot_matrix_clear_buffer(buffer);
  dot_matrix_render_to_buffer(buffer, 6, dm_number(decimal));
}

// todo: refactor
void dot_matrix_render_paused(uint8_t *buffer, int frame) {
  dot_matrix_clear_buffer(buffer);
  dot_matrix_render_to_buffer(buffer, 16 - frame, &dm_P);
  dot_matrix_render_to_buffer(buffer, 20 - frame, &dm_A);
  dot_matrix_render_to_buffer(buffer, 24 - frame, &dm_U);
  dot_matrix_render_to_buffer(buffer, 28 - frame, &dm_S);
  dot_matrix_render_to_buffer(buffer, 32 - frame, &dm_E);
  dot_matrix_render_to_buffer(buffer, 36 - frame, &dm_D);
}

void dot_matrix_render_report(uint8_t *buffer, int frame) {
  dot_matrix_clear_buffer(buffer);
  dot_matrix_render_to_buffer(buffer, 16 - frame, &dm_R);
  dot_matrix_render_to_buffer(buffer, 20 - frame, &dm_E);
  dot_matrix_render_to_buffer(buffer, 24 - frame, &dm_P);
  dot_matrix_render_to_buffer(buffer, 28 - frame, &dm_0);
  dot_matrix_render_to_buffer(buffer, 32 - frame, &dm_R);
  dot_matrix_render_to_buffer(buffer, 36 - frame, &dm_T);
}

void dot_matrix_render_finished(uint8_t *buffer, int frame) {
  dot_matrix_clear_buffer(buffer);
  dot_matrix_render_to_buffer(buffer, 16 - frame, &dm_F); // 4
  dot_matrix_render_to_buffer(buffer, 20 - frame, &dm_I); // 2
  dot_matrix_render_to_buffer(buffer, 22 - frame, &dm_N); // 4
  dot_matrix_render_to_buffer(buffer, 26 - frame, &dm_I); // 2
  dot_matrix_render_to_buffer(buffer, 28 - frame, &dm_S); // 4
  dot_matrix_render_to_buffer(buffer, 32 - frame, &dm_H); // 4
  dot_matrix_render_to_buffer(buffer, 36 - frame, &dm_E); // 4
  dot_matrix_render_to_buffer(buffer, 40 - frame, &dm_D); // 4
}

void dot_matrix_render_text(uint8_t *buffer, char *text, int frame) {
  int start_x = 16;
  dot_matrix_clear_buffer(buffer);
  while (*text) {
    const digit_t *dm = dm_digit(*text);
    if (dm == 0) break;
      dot_matrix_render_to_buffer(buffer, start_x - frame, dm);
      start_x += (dm->cols + 1);
      text++;
  }
}
