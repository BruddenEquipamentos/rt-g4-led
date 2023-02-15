#include "dm_render.h"

#include <string.h>

#include "../panel/rt_led.h"
#include "../treadmill_motor_modbus.h"
#include "dm.h"

static void dm_render_mb_param_value(treadmill_t *treadmill) {
  mb_operation_t *op = modbus_get_current_selected_param();
  dm_render_text(op->rfmtd, 16);
}

void dm_render(treadmill_t *treadmill) {
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
      }
      break;
    }
    case SCENE_PAUSED: {
      dm_render_text_animated("PAUSED", 100, treadmill->get_time_ms());
      break;
    }
    case SCENE_LUB: {
      dm_render_text("LUB", 15);
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
          dm_render_text("S/BL", 16);
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
}
