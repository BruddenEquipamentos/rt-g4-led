#include "dm.h"
void (*_dm_update)(const uint16_t dot_matrix[5]) = 0;

uint16_t dm_buffer[5];
static int frame = 0;
static uint32_t frametime = 100;
static uint32_t frametick = 0;

static void dm_render_to_buffer(const digit_t *digit, int x) {
  for (int row = 0; row < 5; ++row) {
    for (int k = digit->cols - 1, dx = 0; k >= 0; --k, ++dx) {
      if (x + dx >= 16 || x + dx < 0) continue;
      if ((digit->data[row] >> k) & 1) {
        dm_buffer[row] = dm_buffer[row] | (1 << (15 - (x + dx)));
      } else {
        dm_buffer[row] &= ~(0 << (15 - (x + dx)));
      }
    }
  }
}

static void dm_tick(const uint32_t ms) {
  if (ms - frametick > frametime) {
    frametick = ms;
    frame++;
  }
}

void dm_reset_frame(void) {
  frame = 0;
  frametick = 0;
}


void dm_update(void) {
  if (_dm_update != 0) _dm_update(dm_buffer);
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

void dm_render_text_animated(char *text, const uint32_t ms_per_frame, const uint32_t ms) {
  frametime = ms_per_frame;
  dm_tick(ms);
  int length = dm_compute_length(text);
  dm_render_text(text, frame);
  if (frame > length + 16) frame = 0;
}

void dm_clear_buffer(void) {
  for (int i = 0; i < 5; ++i) dm_buffer[i] = 0x0000;
}

void dm_init() { dm_clear_buffer(); }

void dm_render_column_line(int col) {
  if (col > 15) return;
  for (int row = 0; row < 5; ++row) {
    dm_buffer[row] |= (1 << (15 - col));
  }
}

void dm_render_number(int decimal) {
  dm_clear_buffer();
  dm_render_to_buffer(dm_number(decimal), 6);
}
