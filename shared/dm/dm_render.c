#include "dm_render.h"

#include <string.h>

#include "../panel/rt_led.h"
#include "../treadmill_motor_modbus.h"
#include "dm.h"

static void dm_render_mb_param_value(treadmill_t *treadmill) {
  mb_operation_t *op = modbus_get_current_selected_param();
  dm_render_text(op->rfmtd, 16);
}

static uint32_t dm_blink_time_ms = 0;
static int dm_blink(const uint32_t update_time_ms,
                    const uint32_t blink_interval_ms) {
  if (blink_interval_ms == 0) return 1;
  if (update_time_ms - dm_blink_time_ms > blink_interval_ms) {
    dm_blink_time_ms = update_time_ms;
    return 1;
  } else if (update_time_ms - dm_blink_time_ms > blink_interval_ms / 2) {
    return 0;
  }
  return 1;
}

void dm_render(treadmill_t *treadmill) {
  if (treadmill->message.tick) {
    dm_clear_buffer();
    if (!dm_blink(treadmill->get_time_ms(),
                  treadmill->message.blink_interval)) {
      dm_render_text(treadmill->message.data, 16);
    }
    return;
  }

  if (treadmill->scene != treadmill->previous_scene) {
    switch (treadmill->scene) {
      case SCENE_PAUSED: {
        dm_reset_frame();
        break;
      }
      case SCENE_REPORT: {
        dm_reset_frame();
      }
      default:;
    }
  }

  treadmill->previous_scene = treadmill->scene;

  switch (treadmill->scene) {
    case SCENE_IDLE: {
      if (treadmill->config.modbus) {
        dm_render_mb_param_value(treadmill);
      } else {
        dm_clear_buffer();
      }
      break;
    }
    case SCENE_TRAINING: {
      if (treadmill->config.modbus) {
        dm_render_mb_param_value(treadmill);
      } else {
        dm_clear_buffer();

        extern uint8_t __new_display;
        if (__new_display) {
          // if incline has been pressed, show incline for specific time
          if (treadmill->training.incline_changed_time_ms &&
              treadmill->get_time_ms() -
                      treadmill->training.incline_changed_time_ms <=
                  2000) {
            char text[16];
            sprintf(text, "%1d.%1d%%", treadmill->training.incline / 10,
                    treadmill->training.incline % 10);
            dm_render_text(text, 16);
          }
        }

        if (treadmill->program.id == PROGRAM_FAT_BURN ||
            treadmill->program.id == PROGRAM_SUPER_CARDIO) {
          if (treadmill->training.bpm == 0) {
            if (!dm_blink(treadmill->get_time_ms(), 1000)) {
              dm_render_text("@;", 12);
            }
          }
        }
      }
      break;
    }
    case SCENE_EMERGENCY: {
      dm_render_text_animated("EMERGENCIA", 100, treadmill->get_time_ms());
      break;
    }
    case SCENE_PAUSED: {
      dm_render_text_animated("PAUSED", 100, treadmill->get_time_ms());
      break;
    }
    case SCENE_LUB: {
      dm_render_text("LUB", 13);
      break;
    }
    case SCENE_CONFIG: {
      switch (treadmill->config.option) {
        case CONFIG_OPTION_MODEL: {
          dm_render_text("MDL", 16);
          break;
        }  // MDL
        case CONFIG_OPTION_LANGUAGE: {
          dm_render_text("LNG", 16);
          break;
        }  // LNG
        case CONFIG_OPTION_UNIT: {
          dm_render_text("UNT", 16);
          break;
        }  // UNT
        case CONFIG_OPTION_WEIGHT: {
          dm_render_text("WGT", 16);
          break;
        }  // WGT
        case CONFIG_OPTION_MA: {
          dm_render_text("MA", 16);
          break;
        }  // MA
        case CONFIG_OPTION_LUB: {
          dm_render_text("LUB", 16);
          break;
        }  // LUB
        case CONFIG_OPTION_VER: {
          dm_render_text("VER", 16);
          break;
        }  // VER
        case CONFIG_OPTION_TOTAL_DISTANCE: {
          dm_render_text("TD", 16);
          break;
        }  // TD
        case CONFIG_OPTION_TOTAL_HOURS: {
          dm_render_text("THR", 16);
          break;
        }  // THR
        case CONFIG_OPTION_BLE_STATE: {
          if (treadmill->hw.ble.state == BLE_STATE_READY) {
            dm_render_text("BLE", 16);
          } else {
            dm_render_text("S/BL", 16);
          }
          break;
        }  // S/BL
        case CONFIG_OPTION_MODBUS: {
          dm_render_text("MBUS", 16);
          break;
        }  // MBUS
        case CONFIG_OPTION_TEST: {
          dm_render_text("TEST", 16);
          break;
        }  // TEST
        default:;
      }
      break;
    }
    case SCENE_PROGRAM: {
      dm_render_text("PROG", 16);
      break;
    }
    case SCENE_REPORT: {
      dm_render_text_animated("RELATORIO", 100, treadmill->get_time_ms());
      break;
    }
    default: {
      if (treadmill->scene == SCENE_COUNTDOWN &&
          treadmill->countdown.value != 0) {
        dm_render_number(treadmill->countdown.value);
      } else {
        dm_clear_buffer();
      }
    };
  }

  // static int col = 0;
  // static uint32_t t = 0;
  // if (treadmill->get_time_ms() - t >= 250) {
  //   col = (col + 1) % 16;
  //   t = treadmill->get_time_ms();
  // }
  // dm_clear_buffer();
  // for (int i = 0; i < col + 1; ++i) dm_render_column_line(i);
}
