#include "display.h"

#include "digits.h"
#include "dm/dm.h"
#include "main.h"
#include "render.h"
#include "sd/sd.h"
#include "sd/sd_render.h"
#include <string.h>

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

// typedef enum {
//   DOT_POSITION_NONE = 0x00,
//   DOT_POSITION_0 = 0x01,
//   DOT_POSITION_1 = 0x02,
//   DOT_POSITION_2 = 0x04,
//   DOT_POSITION_3 = 0x08,
// } dot_position_t;

typedef struct sdhw {
  uint8_t on_addr;
  uint8_t addr[8];
  dot_position_t dot_pos;
  sd_t* sd;
} sdhw_t;

static sdhw_t display_one = {
    .on_addr = 0x10,
    .addr = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5, 0x06, 0x07},
    .sd = &sd_distance};
static sdhw_t display_two = {
    .on_addr = 0x01,
    .addr = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5, 0x06, 0x07},
    .sd = &sd_time};
static sdhw_t display_three = {
    .on_addr = 0x10,
    .addr = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    .sd = &sd_speed};
static sdhw_t display_four = {
    .on_addr = 0x01,
    .addr = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},
    .sd = &sd_kcal};
static sdhw_t display_five = {
    .on_addr = 0x10,
    .addr = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},
    .sd = &sd_program};
static sdhw_t display_six = {
    .on_addr = 0x01,
    .addr = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    .sd = &sd_pace};
static sdhw_t display_seven = {
    .on_addr = 0x20,
    .addr = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    .sd = &sd_incline};
static sdhw_t display_eight = {
    .on_addr = 0x20,
    .addr = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27},
    .sd = &sd_bpm};

static sdhw_t* sdhw_distance = &display_one;
static sdhw_t* sdhw_time = &display_two;
static sdhw_t* sdhw_speed = &display_three;
static sdhw_t* sdhw_kcal = &display_four;
static sdhw_t* sdhw_program = &display_five;
static sdhw_t* sdhw_pace = &display_six;
static sdhw_t* sdhw_incline = &display_seven;
static sdhw_t* sdhw_bpm = &display_eight;

void display_init(void) {
  uint8_t cmdDriveMode[] = {DISPLAY_CMD_DRIVEMODE, DISPLAY_CMD_DRIVEMODE_CFG};
  uint8_t cmdPwm[] = {DISPLAY_CMD_PWM, DISPLAY_CMD_PWM_CFG};
  uint8_t cmdSystemModeOn[] = {DISPLAY_CMD_SYSTEMMODE,
                               DISPLAY_CMD_SYSTEMMODE_ON};

  // attach to display structure sd_t

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

static void render_to_display(sdhw_t* sdhw, sdigit_t digit, uint8_t pos,
                              uint8_t* buffer) {
  for (uint8_t i = 0; i < 7; ++i) {
    if (segment_digit[digit] & (1 << (7 - i))) {
      buffer[sdhw->addr[i]] |= (sdhw->on_addr << pos);
    } else {
      buffer[sdhw->addr[i]] &= ~(sdhw->on_addr << pos);
    }
  }
}

static void render_dot(sdhw_t* sdhw, uint8_t* buffer) {
  switch (sdhw->on_addr) {
    case 0x01:
      buffer[sdhw->addr[7]] = (buffer[sdhw->addr[7]] & 0xF0) | (sdhw->dot_pos);
      break;
    case 0x10:
      buffer[sdhw->addr[7]] =
          (buffer[sdhw->addr[7]] & 0x0F) | (sdhw->dot_pos << 4);
      break;
    case 0x20:
      buffer[sdhw->addr[7]] =
          (buffer[sdhw->addr[7]] & 0x1F) | (sdhw->dot_pos << 4);
      break;
  }
}

static void render_display_clear(const sdhw_t* sdhw, uint8_t* buffer) {
  for (uint8_t i = 0; i < 8; ++i) {
    switch (sdhw->on_addr) {
      case 0x01:
        buffer[sdhw->addr[i]] &= 0xF0;
        break;
      case 0x10:
        buffer[sdhw->addr[i]] &= 0x0F;
        break;
      case 0x20:
        buffer[sdhw->addr[i]] &= 0x1F;
        break;
    }
  }
}

static void render_display_distance(sdhw_t* sdhw, uint8_t* buffer) {
  render_display_clear(sdhw, buffer);
  sdhw->dot_pos = DOT_POSITION_2;
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[3] > DIGIT_0 && sdhw->sd->digit[3])
    render_to_display(sdhw, sdhw->sd->digit[3], 3, buffer);
  render_to_display(sdhw, sdhw->sd->digit[2], 2, buffer);
  render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 0, buffer);
  render_dot(sdhw, buffer);
}

static void render_display_time(sdhw_t *sdhw, uint8_t* buffer) {
  // 0:00
  render_display_clear(sdhw, buffer);
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[3] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[3], 3, buffer);
  render_to_display(sdhw, sdhw->sd->digit[2], 2, buffer);
  render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 0, buffer);
  render_dot(sdhw, buffer);
}

static void render_display_speed(sdhw_t *sdhw, uint8_t* buffer) {
  // 1.0
  render_display_clear(sdhw, buffer);
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[3] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[3], 3, buffer);
  if (sdhw->sd->digit[2] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[2], 2, buffer);
  render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 0, buffer);
  sdhw->dot_pos = DOT_POSITION_1;
  render_dot(sdhw, buffer);
}

static void render_display_program(sdhw_t *sdhw, uint8_t* buffer) {
  // 0, 10, 100, 1000
  render_display_clear(sdhw, buffer);
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[3] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[3], 3, buffer);
  if (sdhw->sd->digit[3] > DIGIT_0 || sdhw->sd->digit[2] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[2], 2, buffer);
  if (sdhw->sd->digit[2] > DIGIT_0 || sdhw->sd->digit[1] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 0, buffer);
  render_dot(sdhw, buffer);
}

static void render_display_calories(sdhw_t *sdhw, uint8_t* buffer) {
  // 0, 10, 100, 1000
  render_display_clear(sdhw, buffer);
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[3] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[3], 3, buffer);
  if (sdhw->sd->digit[3] > DIGIT_0 || sdhw->sd->digit[2] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[2], 2, buffer);
  if (sdhw->sd->digit[2] > DIGIT_0 || sdhw->sd->digit[1] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 0, buffer);
  render_dot(sdhw, buffer);
}

static void render_display_pace(sdhw_t *sdhw, uint8_t* buffer) {
  // 0, 10, 100, 1000
  render_display_clear(sdhw, buffer);
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[3] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[3], 3, buffer);
  // if (sdhw->sd->digit[3] > DIGIT_0 || sdhw->sd->digit[2] > DIGIT_0)
  render_to_display(sdhw, sdhw->sd->digit[2], 2, buffer);
  // if (sdhw->sd->digit[2] > DIGIT_0 || sdhw->sd->digit[1] > DIGIT_0)
  render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 0, buffer);
  render_dot(sdhw, buffer);
}

static void render_display_incline(sdhw_t *sdhw, uint8_t* buffer) {
  // 0, 10, 100 -- digit are reversed
  render_display_clear(sdhw, buffer);
  sdhw->dot_pos = DOT_POSITION_2;
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[2] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[2], 0, buffer);
  // if (sdhw->sd->digit[2] > DIGIT_0 || sdhw->sd->digit[1] > DIGIT_0)
  render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 2, buffer);
  render_dot(sdhw, buffer);
}

static void render_display_bpm(sdhw_t *sdhw, uint8_t* buffer) {
  // 0, 10, 100 -- digit are reversed
  render_display_clear(sdhw, buffer);
  if (!sdhw->sd->visible) return;
  if (sdhw->sd->digit[2] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[2], 0, buffer);
  if (sdhw->sd->digit[2] > DIGIT_0 || sdhw->sd->digit[1] > DIGIT_0)
    render_to_display(sdhw, sdhw->sd->digit[1], 1, buffer);
  render_to_display(sdhw, sdhw->sd->digit[0], 2, buffer);
  render_dot(sdhw, buffer);
}


static void display_update_dm_buffer(uint8_t* buffer,
                                             uint16_t dm_buffer[5]) {
  static const uint8_t column_address[16] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
                                             0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
                                             0x24, 0x25, 0x26, 0x27};

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

void display_update(void) {
  static uint8_t buffer[0x32] = {DISPLAY_CMD_PAGEWRITE, 0x00};
  static uint8_t buffer_sent[0x32] = {0x00};
  static uint8_t* page_buffer = &buffer[2];

  render_display_distance(sdhw_distance, page_buffer);
  render_display_time(sdhw_time, page_buffer);
  render_display_program(sdhw_program, page_buffer);
  render_display_calories(sdhw_kcal, page_buffer);
  render_display_incline(sdhw_incline, page_buffer);
  render_display_bpm(sdhw_bpm, page_buffer);
  render_display_pace(sdhw_pace, page_buffer);
  render_display_speed(sdhw_speed, page_buffer);

  // convert buffer to page buffer mapping
  display_update_dm_buffer(page_buffer, dm_buffer);

  if (memcmp(buffer_sent, buffer, 0x32) != 0) {
    if (HAL_I2C_Master_Transmit(&hi2c1, DISPLAY_ADDRESS_WRITE, buffer, BUFSIZE,
                                100) == HAL_OK) {
      memcpy(buffer_sent, buffer, 0x32);
    }
  }
}
