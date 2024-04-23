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

int dm_render_text_animated(char *text, const uint32_t ms_per_frame, const uint32_t ms) {
  frametime = ms_per_frame;
  dm_tick(ms);
  int length = dm_compute_length(text);
  dm_render_text(text, frame);
  if (frame > length + 16) {
    frame = 0;
    return 1;
  }
  return 0;
}

void dm_clear_buffer(void) {
  for (int i = 0; i < 5; ++i) dm_buffer[i] = 0x0000;
}

void dm_init(void) { dm_clear_buffer(); }

void dm_render_column_line(int col) {
  if (col > 15) return;
  for (int row = 0; row < 5; ++row) {
    dm_buffer[row] |= (1 << (15 - col));
  }
}

void dm_render_column_with_height(int col, int height, uint32_t current_ms, uint32_t blink_ms) {
  static uint32_t dm_blink_time_ms = 0;
  if (blink_ms != 0) {
    if (current_ms - dm_blink_time_ms > blink_ms) {
      dm_blink_time_ms = current_ms;
    } else if (current_ms - dm_blink_time_ms > blink_ms / 2) {
      return;
    }
  }
  for (int i = 0; i < 5 && height >= 0; ++i) {
    dm_buffer[4 - i] |= (1 << (15 - col));
    height--;
  }
}

// dm_render_line(0, 4, 15, 4);
void dm_render_line(int line) {
  /*
  4 ................
  3 ................
  2 ................
  1 ................
  0 ................
  */
  if (line > 4) return;
  for (int col = 0; col < 16; col++) {
    dm_buffer[line] |= (1 << (15 - col));
  }
}

void dm_render_number(int decimal) {
  dm_clear_buffer();
  dm_render_to_buffer(dm_number(decimal), 6);
}

struct dm_buffer {    
    uint16_t data[5];
};

struct dm_buffer dm_road_buffer[9] = {  
    {.data = {
        0b0000001111000000,
        0b0000011111100000,
        0b0000111111110000,
        0b0001111111111000,
        0b0011111111111100,
    }},
    {.data = {
        0b0000000000000000,
        0b0000011111100000,
        0b0000111111110000,
        0b0001111111111000,
        0b0011111111111100,
    }},
    {.data = {
        0b0000000000000000,
        0b0000010000100000,
        0b0000111111110000,
        0b0001111111111000,
        0b0011111111111100,
    }},
    {.data = {
        0b0000001001000000,
        0b0000010000100000,
        0b0000110000110000,
        0b0001111111111000,
        0b0011111111111100,
    }},
    {.data = {
        0b0000001001000000,
        0b0000011001100000,
        0b0000110000110000,
        0b0001110000111000,
        0b0011111111111100,
    }},
    {.data = {
        0b0000001111000000,
        0b0000011001100000,
        0b0000110000110000,
        0b0001110000111000,
        0b0011110000111100,
    }},
    {.data = {
        0b0000001111000000,
        0b0000011111100000,
        0b0000111001110000,
        0b0001111001111000,
        0b0011110000111100,
    }},
    {.data = {
        0b0000001111000000,
        0b0000011111100000,
        0b0000111111110000,
        0b0001111001111000,
        0b0011111001111100,
    }},
    {.data = {
        0b0000001111000000,
        0b0000011111100000,
        0b0000111111110000,
        0b0001111111111000,
        0b0011111001111100,
    }},
};

void dm_render_road(uint32_t ms_per_frame, uint32_t ms) {
  frametime = ms_per_frame;
  dm_tick(ms);
  if (frame < 9) {
    memcpy(dm_buffer, dm_road_buffer[frame].data, sizeof(uint16_t)*5);  
  } else {
    frame = 0;    
  }  
}