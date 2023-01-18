/*
 * display.c
 *
 *  Created on: Feb 1, 2021
 *      Author: movement
 */

#include "display.h"

#include <string.h>

#include "d_dot_matrix.h"

#include "digits.h"
#include "main.h"
#include "render.h"

// #define DISPLAY_LOAD_LEVEL_IN_POWER_FIELD

#define BUFSIZE 0x2A  // 0x32

extern I2C_HandleTypeDef hi2c1;

static void render_to_display(display_t* display, sdigit_t digit, uint8_t pos,
                              uint8_t* buffer) {
  for (uint8_t i = 0; i < 7; ++i) {
    if (segment_digit[digit] & (1 << (7 - i))) {
      buffer[display->addr[i]] |= (display->on_addr << pos);
    } else {
      buffer[display->addr[i]] &= ~(display->on_addr << pos);
    }
  }
}

static inline void render_dot(display_t* display, uint8_t* buffer) {
  switch (display->on_addr) {
    case 0x01:
      buffer[display->addr[7]] =
          (buffer[display->addr[7]] & 0xF0) | (display->dot_pos);
      break;
    case 0x10:
      buffer[display->addr[7]] =
          (buffer[display->addr[7]] & 0x0F) | (display->dot_pos << 4);
      break;
    case 0x20:
      buffer[display->addr[7]] =
          (buffer[display->addr[7]] & 0x1F) | (display->dot_pos << 4);
      break;
  }
}

static void render_display_clear(const display_t* d, uint8_t* buffer) {
  for (uint8_t i = 0; i < 8; ++i) {
    switch (d->on_addr) {
      case 0x01:
        buffer[d->addr[i]] &= 0xF0;
        break;
      case 0x10:
        buffer[d->addr[i]] &= 0x0F;
        break;
      case 0x20:
        buffer[d->addr[i]] &= 0x1F;
        break;
    }
  }
}

static void render_display(display_t* display, uint8_t* buffer) {
  render_display_clear(display, buffer);
  if (!display->visible) return;
  render_to_display(display, display->digits[3], 3, buffer);
  render_to_display(display, display->digits[2], 2, buffer);
  render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 0, buffer);
  render_dot(display, buffer);
}

// TODO: indicate that is related to format
static void render_display_distance(display_t* display, uint8_t* buffer) {
  // 0.50 (500 m)
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[3] > DIGIT_0 && display->digits[3])
    render_to_display(display, display->digits[3], 3, buffer);

  render_to_display(display, display->digits[2], 2, buffer);
  render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 0, buffer);
  render_dot(display, buffer);
}

static void render_display_time(display_t* display, uint8_t* buffer) {
  // 0:00
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[3] > DIGIT_0)
    render_to_display(display, display->digits[3], 3, buffer);
  render_to_display(display, display->digits[2], 2, buffer);
  render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 0, buffer);
  render_dot(display, buffer);
}

static void render_display_speed(display_t* display, uint8_t* buffer) {
  // 1.0
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[3] > DIGIT_0)
    render_to_display(display, display->digits[3], 3, buffer);
  if (display->digits[2] > DIGIT_0)
    render_to_display(display, display->digits[2], 2, buffer);
  render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 0, buffer);
  display->dot_pos = DOT_POSITION_1;
  render_dot(display, buffer);
}

static void render_display_steps(display_t* display, uint8_t* buffer) {
  // 0, 10, 100, 1000
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[3] > DIGIT_0)
    render_to_display(display, display->digits[3], 3, buffer);
  if (display->digits[3] > DIGIT_0 || display->digits[2] > DIGIT_0)
    render_to_display(display, display->digits[2], 2, buffer);
  if (display->digits[2] > DIGIT_0 || display->digits[1] > DIGIT_0)
    render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 0, buffer);
  render_dot(display, buffer);
}

static void render_display_calories(display_t* display, uint8_t* buffer) {
  // 0, 10, 100, 1000
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[3] > DIGIT_0)
    render_to_display(display, display->digits[3], 3, buffer);
  if (display->digits[3] > DIGIT_0 || display->digits[2] > DIGIT_0)
    render_to_display(display, display->digits[2], 2, buffer);
  if (display->digits[2] > DIGIT_0 || display->digits[1] > DIGIT_0)
    render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 0, buffer);
  render_dot(display, buffer);
}

static void render_display_spm(display_t* display, uint8_t* buffer) {
  // 0, 10, 100, 1000
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[3] > DIGIT_0)
    render_to_display(display, display->digits[3], 3, buffer);
  if (display->digits[3] > DIGIT_0 || display->digits[2] > DIGIT_0)
    render_to_display(display, display->digits[2], 2, buffer);
  if (display->digits[2] > DIGIT_0 || display->digits[1] > DIGIT_0)
    render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 0, buffer);
  render_dot(display, buffer);
}

static void render_display_power(display_t* display, uint8_t* buffer) {
  // 0, 10, 100 -- digits are reversed
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[2] > DIGIT_0)
    render_to_display(display, display->digits[2], 0, buffer);
  if (display->digits[2] > DIGIT_0 || display->digits[1] > DIGIT_0)
    render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 2, buffer);
  render_dot(display, buffer);
}

static void render_display_bpm(display_t* display, uint8_t* buffer) {
  // 0, 10, 100 -- digits are reversed
  render_display_clear(display, buffer);
  if (!display->visible) return;
  if (display->digits[2] > DIGIT_0)
    render_to_display(display, display->digits[2], 0, buffer);
  if (display->digits[2] > DIGIT_0 || display->digits[1] > DIGIT_0)
    render_to_display(display, display->digits[1], 1, buffer);
  render_to_display(display, display->digits[0], 2, buffer);
  render_dot(display, buffer);
}

void set_display_pwm_level(const uint8_t level) {
  uint8_t cmdPwm[] = {DISPLAY_CMD_PWM, DISPLAY_CMD_PWM_CFG};
  cmdPwm[1] = level;
  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdPwm,
                              sizeof(cmdPwm), 1000) != HAL_OK) {
    // throw error
  }
}

static void display_helper_render_test(display_t* display, uint8_t value) {
  display->dot_pos = DOT_POSITION_NONE;
  display->digits[3] = value % 10;
  display->digits[2] = value % 10;
  display->digits[1] = value % 10;
  display->digits[0] = value % 10;
}

static void display_update_dot_matrix_buffer(uint8_t *buffer, uint16_t dm_buffer[5]){
  static const uint8_t column_address[16] = {
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};

  static const uint8_t row_value[5] = {
      0x01, 0x02, 0x04, 0x08, 0x10};

  for (int row = 0; row < 5; ++row) {
      for (int col = 15; col >= 0; --col) {
	 if ((dm_buffer[row]>>col) & 1) buffer[column_address[15-col]] |= row_value[row];
	 else buffer[column_address[15-col]] &= ~row_value[row];
      }
  }
}

static void display_render_test(uint8_t* buffer, uint8_t* page_buffer) {
  static uint8_t value = 0;
  display_helper_render_test(display_distance, value);
  display_helper_render_test(display_time, value);
  display_helper_render_test(display_speed, value);
  display_helper_render_test(display_steps, value);
  display_helper_render_test(display_calories, value);
  display_helper_render_test(display_spm, value);
  display_helper_render_test(display_power, value);
  display_helper_render_test(display_bpm, value);
  value = (value + 1) % 10;

  //dot_matrix_render_column_line(15);

  static uint8_t column = 0;
  if (column == 16) {
    column = 0;
    dot_matrix_clear_buffer();
  }
  dot_matrix_render_column_line(column);
  column = (column + 1);
}

void display_render(void) {
  static uint8_t buffer[0x32] = {DISPLAY_CMD_PAGEWRITE, 0x00};
  static uint8_t buffer_sent[0x32] = {0x00};
  static uint8_t* page_buffer = &buffer[2];

  display_render_test(buffer, page_buffer);

  // render_display_distance(display_distance, page_buffer);
  render_display(display_distance, page_buffer);
  render_display(display_time, page_buffer);
  render_display(display_speed, page_buffer);
  render_display(display_steps, page_buffer);
  render_display(display_calories, page_buffer);
  render_display(display_spm, page_buffer);
  render_display(display_power, page_buffer);
  render_display(display_bpm, page_buffer);

  // convert buffer to page buffer mapping
  display_update_dot_matrix_buffer(page_buffer, dot_matrix_buffer);

  if (memcmp(buffer_sent, buffer, 0x32) != 0) {
    if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, buffer, BUFSIZE,
                                100) == HAL_OK) {
      memcpy(buffer_sent, buffer, 0x32);
    }
  }
}

void display_init() {
  uint8_t cmdDriveMode[] = {DISPLAY_CMD_DRIVEMODE, DISPLAY_CMD_DRIVEMODE_CFG};
  uint8_t cmdPwm[] = {DISPLAY_CMD_PWM, DISPLAY_CMD_PWM_CFG};
  uint8_t cmdSystemModeOn[] = {DISPLAY_CMD_SYSTEMMODE,
                               DISPLAY_CMD_SYSTEMMODE_ON};

  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdDriveMode,
                              sizeof(cmdDriveMode), 1000) != HAL_OK) {
    // throw error
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdPwm,
                              sizeof(cmdPwm), 1000) != HAL_OK) {
    // throw error
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdSystemModeOn,
                              sizeof(cmdSystemModeOn), 1000) != HAL_OK) {
    // throw error
  }

  //    // blink test
  uint8_t cmdBlink[] = {0x84, 0x00};  // 0 - 0hz, 0.5hz, 1hz, 2hz
  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdBlink,
                              sizeof(cmdBlink), 1000) != HAL_OK) {
    // throw error
  }
}
