/*
 * display.c
 *
 *  Created on: Mar 27, 2023
 *      Author: fminoru
 */

#include "display.h"

#include "main.h"

#define DISPLAY_ADDRESS_WRITE 0xD0
#define DISPLAY_ADDRESS_READ 0xD1
#define DISPLAY_CMD_PAGEWRITE 0x80
#define DISPLAY_CMD_DRIVEMODE 0x82
#define DISPLAY_CMD_DRIVEMODE_CFG 0x02
#define DISPLAY_CMD_SYSTEMMODE 0x82
#define DISPLAY_CMD_SYSTEMMODE_ON 0x03
#define DISPLAY_CMD_BLINK 0x84
#define DISPLAY_CMD_BLINK_CFG 0x00
#define DISPLAY_CMD_PWM 0xC0
#define DISPLAY_CMD_PWM_CFG 0xFF

#define BUFSIZE 0x2A  // 0x32

extern I2C_HandleTypeDef hi2c1;

static uint16_t dm_buffer[5] = {0};

int display_init(void) {
  uint8_t cmdDriveMode[] = {DISPLAY_CMD_DRIVEMODE, DISPLAY_CMD_DRIVEMODE_CFG};
  uint8_t cmdPwm[] = {DISPLAY_CMD_PWM, DISPLAY_CMD_PWM_CFG};
  uint8_t cmdSystemModeOn[] = {DISPLAY_CMD_SYSTEMMODE, DISPLAY_CMD_SYSTEMMODE_ON};

  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdDriveMode, sizeof(cmdDriveMode), 1000) != HAL_OK) {
    return 0;
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdPwm, sizeof(cmdPwm), 1000) != HAL_OK) {
    return 0;
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdSystemModeOn, sizeof(cmdSystemModeOn), 1000) !=
      HAL_OK) {
    return 0;
  }

  uint8_t cmdBlink[] = {0x84, 0x00};  // 0 - 0hz, 0.5hz, 1hz, 2hz
  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, cmdBlink, sizeof(cmdBlink), 1000) != HAL_OK) {
    return 0;
  }

  return 1;
}

int dm_render_column_line(int col) {
  if (col > 15) return 0;
  for (int row = 0; row < 5; ++row) {
    dm_buffer[row] |= (1 << (15 - col));
  }
  return 1;
}

int dm_render_progress(const uint32_t value, const uint32_t total, int foward) {
  uint32_t col = (16 * value) / total;

  dm_render_clear();

  if (col == 0) {
    return 1;
  }

  if (foward) {
    for (int i = 0; i < col; ++i) {
      dm_render_column_line(i);
    }
  } else {
    col = 16 - col;
    for (int i = 0; i < col; ++i) {
      dm_render_column_line(i);
    }
  }

  return 1;
}

int dm_render_progress_dfu(const uint32_t value, const uint32_t total) {

  uint32_t dot = (80 * value) / total;

  for (int row = 0; row < 5; ++row) {
      int lines = dot/16;
      int remainder = dot % 16;

      if (row < lines) {
	  dm_buffer[row] = 0xffff;
      } else {
	  for (int col = 0; col < remainder; col++) {
	      dm_buffer[row] |= (1 << (15-col));
	  }
	  break;
      }
  }
  /*
   * 5 * 16 => 80 dots
   *
   *
   */

}

static void display_update_dm_buffer(uint8_t* buffer, uint16_t dm_buffer[5]) {
  static const uint8_t column_address[16] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                             0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};

  static const uint8_t row_value[5] = {0x01, 0x02, 0x04, 0x08, 0x10};

  for (int row = 0; row < 5; ++row) {
    for (int col = 15; col >= 0; --col) {
      if ((dm_buffer[row] >> col) & 1)
        buffer[column_address[15 - col]] |= row_value[row];
      else
        buffer[column_address[15 - col]] &= ~row_value[row];
    }
  }
}

int dm_render_clear(void) {
  for (int i = 0; i < 5; i++) {
    dm_buffer[i] = 0x0000;
  }
  return 1;
}

int display_update(void) {
  static uint8_t buffer[0x32] = {DISPLAY_CMD_PAGEWRITE, 0x00};
  static uint8_t buffer_sent[0x32] = {0x00};
  static uint8_t* display_buffer = &buffer[2];
  //  }

  display_update_dm_buffer(display_buffer, dm_buffer);

  if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, buffer, BUFSIZE, 100) == HAL_OK) {
    // memcpy(buffer_sent, buffer, 0x32);
  }

  return 1;
}
