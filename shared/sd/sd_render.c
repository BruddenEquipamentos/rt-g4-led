#include "sd_render.h"

#include "../panel/render_program_scene.h"
#include "../panel/rt_led.h"
#include "../treadmill_motor_modbus.h"
#include <stdbool.h>
/*
// display (iLed)
distance time speed
calories program pace
incline ... bpm

// display (led)
distance time speed
calories pace bpm
*/
extern int report_state;
extern uint8_t config_error_index;

sd_t sd_distance = {.flag = SD_FLAG_VISIBLE};
sd_t sd_time = {.flag = SD_FLAG_VISIBLE};
sd_t sd_speed = {.flag = SD_FLAG_VISIBLE};
sd_t sd_kcal = {.flag = SD_FLAG_VISIBLE};
sd_t sd_program = {.flag = SD_FLAG_VISIBLE};
sd_t sd_pace = {.flag = SD_FLAG_VISIBLE};
sd_t sd_incline = {.flag = SD_FLAG_VISIBLE};
sd_t sd_bpm = {.flag = SD_FLAG_VISIBLE};

void sd_render_all_visible(bool visible) {
  sd_set_visible(&sd_distance, visible);
  sd_set_visible(&sd_time, visible);
  sd_set_visible(&sd_speed, visible);
  sd_set_visible(&sd_kcal, visible);
  sd_set_visible(&sd_program, visible);
  sd_set_visible(&sd_pace, visible);
  sd_set_visible(&sd_incline, visible);
  sd_set_visible(&sd_bpm, visible);
}

static void sd_render_all_no_pad(void) {
  sd_set_pad_leading_zero(&sd_distance, false);
  sd_set_pad_leading_zero(&sd_time, false);
  sd_set_pad_leading_zero(&sd_speed, false);
  sd_set_pad_leading_zero(&sd_kcal, false);
  sd_set_pad_leading_zero(&sd_program, false);
  sd_set_pad_leading_zero(&sd_pace, false);
  sd_set_pad_leading_zero(&sd_incline, false);
  sd_set_pad_leading_zero(&sd_bpm, false);
}

static void sd_set_value_to_all(const uint32_t value) {
  sd_set_value(&sd_distance, value);
  sd_set_value(&sd_time, value);
  sd_set_value(&sd_speed, value);
  sd_set_value(&sd_kcal, value);
  sd_set_value(&sd_program, value);
  sd_set_value(&sd_pace, value);
  sd_set_value(&sd_incline, value);
  sd_set_value(&sd_bpm, value);
}

extern uint8_t speed_changed;
extern uint8_t incline_changed;

uint8_t aux_teste = 0;

static char sd_hex_nibble_to_char(uint8_t nibble) {
  nibble &= 0x0Fu;
  if (nibble < 10u) {
    return (char)('0' + nibble);
  }

  switch (nibble) {
    case 10u: return 'A';
    case 11u: return 'b';
    case 12u: return 'C';
    case 13u: return 'd';
    case 14u: return 'E';
    case 15u: return 'f';
    default: return '0';
  }
}

static void sd_render_idle_error(treadmill_t *treadmill) {
  uint8_t error_code = treadmill_get_latest_error_code(treadmill);
  char error_text[5] = {' ', 'E', '0', '0', '\0'};

  if (error_code == 0u) {
    sd_set_visible(&sd_program, false);
    return;
  }

  error_text[2] = sd_hex_nibble_to_char(error_code >> 4);
  error_text[3] = sd_hex_nibble_to_char(error_code);
  sd_set_visible(&sd_program, true);
  sd_set_pad_leading_zero(&sd_program, false);
  sd_program.dot = SD_DOT_NO_DOT;
  sd_set_text(&sd_program, error_text);
}

static void render_training_data(treadmill_t *treadmill) {
  sd_set_km_m(&sd_distance, treadmill->training.meters);
  sd_set_min_sec(&sd_time, treadmill->training.seconds);
  sd_set_kmh(&sd_speed, treadmill->training.speed);
  sd_set_min_sec(&sd_pace, treadmill->training.pace);
  sd_set_kcal(&sd_kcal, treadmill->training.kcal / 1000);
  sd_set_incline(&sd_incline, treadmill->training.incline);


  if (treadmill->scene == SCENE_TRAINING) {
    if ((speed_changed == 1) && treadmill->get_time_ms() - treadmill->key_pressed_last_tick <= 2000){
      sd_render_all_visible(false);
      sd_set_visible(&sd_speed, true);
    }
    else{
      speed_changed = 0;
    }
    if ((incline_changed == 1) && treadmill->get_time_ms() - treadmill->key_pressed_last_tick <= 2000){
      sd_render_all_visible(false);
      if ((treadmill->training.incline == treadmill->config.incline.min) || (treadmill->training.incline == treadmill->config.incline.max)) {
//        aux_teste = 1; //todo: check if incline is min or max if user press incline increment key
        /////////////////////////////////
      }
    }      
    else{
      incline_changed = 0;
    }
  }
  
  // TODO: refactor
  if (treadmill->scene == SCENE_TRAINING && treadmill->aux.speed_reached != treadmill->aux.speed_target) {
    sd_blink(&sd_speed, treadmill->get_time_ms(), 500);
  }

  if (treadmill->scene == SCENE_PAUSED) {
    sd_set_min_sec(&sd_pace, 0);
    sd_set_kmh(&sd_speed, 0);
  }

  // if (treadmill->scene == SCENE_REPORT) { //////////////////////////////
  //   sd_set_min_sec(&sd_pace, treadmill->training.pace_total_seconds / treadmill->training.seconds);
  //   sd_set_bpm(&sd_bpm, treadmill->training.bpm_average);
  //   // on report show average speed
  //   uint32_t cm_sec = (treadmill->training.meters * 100 / treadmill->training.seconds);
  //   uint32_t speed = ((cm_sec * 36) / 10);  // 333 * 36 = 1198
  //   uint32_t speed_mod = speed % 10;
  //   if (speed_mod >= 5) speed = speed + (10 - speed_mod);
  //   sd_set_kmh(&sd_speed, speed);
  // }
    if (treadmill->scene == SCENE_REPORT) {
      if (report_state == 0){
        uint32_t speed_max = treadmill->training.speed_max;
        // sd_set_kmh(&sd_speed, speed_max);
        sd_set_kmh_report(&sd_speed, speed_max);
        sd_set_min_sec(&sd_pace, treadmill->training.pace_max);
        sd_set_bpm(&sd_bpm, treadmill->training.bpm_max);
      }
      else if (report_state == 1) {
        uint32_t speed_avg = treadmill->training.average_speed;
        sd_set_kmh_report(&sd_speed, speed_avg);
        sd_set_min_sec(&sd_pace, treadmill_calculate_pace(speed_avg));
        sd_set_bpm(&sd_bpm, treadmill->training.bpm_average); 
      }
    }

  if (treadmill->config.modbus) {
    sd_set_visible(&sd_program, true);
    char text[5];
    uint16_t value = modbus_current_selected_param_value() % 1000u;
    text[0] = 'P';
    text[1] = (char)('0' + ((value / 100u) % 10u));
    text[2] = (char)('0' + ((value / 10u) % 10u));
    text[3] = (char)('0' + (value % 10u));
    text[4] = '\0';
    sd_set_text(&sd_program, text);
  }

  if (treadmill->hw.handgrip.state == HEARTRATE_HG_INITING) {
    sd_set_bpm_detected(&sd_bpm);
    sd_blink(&sd_bpm, treadmill->get_time_ms(), 500);
  } else if (treadmill->scene != SCENE_REPORT){    
    sd_set_bpm(&sd_bpm, treadmill->training.bpm);
  }
}

static void render_training_data_program(treadmill_t *treadmill) {
  if (treadmill->scene == SCENE_PAUSED) {
    sd_set_min_sec(&sd_pace, 0);
    sd_set_kmh(&sd_speed, 0);
  }
  switch (treadmill->program.id) {
    case PROGRAM_TIME: {
      uint32_t remaining_seconds =  treadmill->program.p_time.seconds - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_DISTANCE: {
      uint32_t remaining_meters = treadmill->program.p_distance.meters - treadmill->training.meters;
      sd_set_km_m(&sd_distance, remaining_meters);
      break;
    }
    case PROGRAM_CALORIES: {
      uint32_t remaining_kcal = treadmill->program.p_calories.kcal - treadmill->training.kcal / 1000;
      sd_set_value(&sd_kcal, remaining_kcal);
      break;
    }
    case PROGRAM_HIIT: {
      uint32_t remaining_seconds = treadmill->program.p_hiit.seconds - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_WALK_30MIN: {
      uint32_t remaining_seconds = (30 * 60) - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_FAT_BURN: {
      uint32_t remaining_seconds = treadmill->program.p_fat_burn.seconds - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_SUPER_CARDIO: {
      uint32_t remaining_seconds = treadmill->program.p_super_cardio.seconds - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_WALK_08MIN: {
      uint32_t remaining_seconds = (8 * 60) - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_200K_CALORIES: {
      uint32_t remaining_kcal = treadmill->program.p_200k_calories.kcal - treadmill->training.kcal / 1000;
      sd_set_value(&sd_kcal, remaining_kcal);
      break;
    }
    default:
      render_training_data(treadmill);
      break;
  }
}

static void render_config_data(treadmill_t *treadmill) {
  sd_render_all_visible(false);
  // options
  switch (treadmill->config.option) {
    case CONFIG_OPTION_MODEL: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.model);
      break;
    }
    case CONFIG_OPTION_NAME: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.device_name_id);
      break;
    }
    case CONFIG_OPTION_LANGUAGE: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.language);
      break;
    }
    case CONFIG_OPTION_UNIT: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.unit);
      break;
    }
    case CONFIG_OPTION_WEIGHT: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.weight);
      break;
    }
    case CONFIG_OPTION_MA: {
      sd_set_visible(&sd_pace, false);
      break;
    }
    case CONFIG_OPTION_VER: {
      sd_set_visible(&sd_pace, true);
      sd_set_version(&sd_pace, MAJOR, MINOR, REVISION);
      break;
    }
    case CONFIG_OPTION_TOTAL_DISTANCE: {
      if ((treadmill->total.distance_in_meters / 1000)/10000) {
    	  sd_set_visible(&sd_program, true);
    	  sd_set_value(&sd_program, (treadmill->total.distance_in_meters / 1000) / 10000);
      }

      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, (treadmill->total.distance_in_meters / 1000) % 10000);

      // sd_program.flag = SD_FLAG_PAD_LEADING_ZERO; //test
      break;
    }
    case CONFIG_OPTION_TOTAL_HOURS: {
      if ((treadmill->total.time_in_seconds / 3600) / 10000) {
    	sd_set_visible(&sd_program, true);
 	    sd_set_value(&sd_program, (treadmill->total.time_in_seconds / 3600) / 10000);
      }

      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, (treadmill->total.time_in_seconds / 3600) % 10000);

      // sd_program.flag = SD_FLAG_PAD_LEADING_ZERO; //test
      break;
    }
    case CONFIG_OPTION_BLE_STATE: {
      sd_set_visible(&sd_pace, false);
      break;
    }
    case CONFIG_OPTION_MODBUS: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.modbus);
      break;
    }
    case CONFIG_OPTION_TEST: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.test);
      break;
    }
    case CONFIG_OPTION_C2V: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.c2v);
      break;
    }
    case CONFIG_OPTION_ERRORS: {
      const uint8_t inverter_last_start = INVERTER_ERROR_COUNT;
      const uint8_t keyboard_count_start = INVERTER_ERROR_COUNT + LAST_ERROR_HISTORY_COUNT;

      sd_set_visible(&sd_pace, true);
      if (config_error_index < INVERTER_ERROR_COUNT) {
        sd_set_value(&sd_pace, treadmill->hw.inverter.error[config_error_index].count);
      }
      else if (config_error_index < keyboard_count_start) {
        uint8_t last_error_index = config_error_index - inverter_last_start;
        if (last_error_index < LAST_ERROR_HISTORY_COUNT) {
          static const char *last_error_logic[LAST_ERROR_HISTORY_COUNT] = {"L01", "L02", "L03"};
          sd_set_pad_leading_zero(&sd_pace, false);
          sd_pace.dot = SD_DOT_NO_DOT;
          sd_set_text(&sd_pace, last_error_logic[last_error_index]);
        }
      } else if (config_error_index < CONFIG_ERRORS_TOTAL_ENTRIES) {
        uint8_t keyboard_error_index = config_error_index - keyboard_count_start;
        sd_set_value(&sd_pace, treadmill->hw.keyboard.error[keyboard_error_index].count);
      } else {
        sd_set_value(&sd_pace, 0);
      }
      break;
    }
    case CONFIG_OPTION_MAX_TIME: {
      sd_set_visible(&sd_time, true);
      if (treadmill->config.max_time < MAX_TIME_ON_TREADMILL_LOWER){
        sd_set_text(&sd_time, "----");
        sd_blink(&sd_time, treadmill->get_time_ms(), 800);
      }
      else{
        sd_set_min_sec(&sd_time, treadmill->config.max_time);
      }
      break;
    }
    default:
      sd_set_visible(&sd_pace, false);
  }
}

void sd_render(treadmill_t *treadmill) {
  sd_render_all_visible(true);
  switch (treadmill->scene) {
    case SCENE_INIT: {
      sd_set_value_to_all(8888);
      sd_render_all_visible(true);
      // sd_set_visible(&sd_program, false);
      // render_training_data(treadmill);
      break;
    }
    case SCENE_IDLE: {
      // extern int run_test;
      // extern int test_count;
      // if (run_test == 1) {
      //   sd_set_value_to_all(run_test);
      // } else if (run_test == 2) {
      //   sd_set_value_to_all(test_count);
      // } else if (run_test == 3) {
      //   sd_set_value_to_all(run_test);
      // } else {
      //   sd_set_visible(&sd_program, false);
      //   render_training_data(treadmill);
      // }

      // sd_set_visible(&sd_program, false);
      sd_render_all_visible(false); 
      // sd_set_value_to_all(8888); ///led test
      render_training_data(treadmill);
      sd_render_idle_error(treadmill);
      
      break;
    }
    case SCENE_BLE_ALLOW_YES_NO: {
      sd_render_all_visible(false);
      render_training_data(treadmill);
      break;
    }
    case SCENE_EMERGENCY: {
      sd_render_all_visible(false);
      break;
    }
    case SCENE_CONFIG: {
      sd_set_visible(&sd_program, false);
      render_config_data(treadmill);
      break;
    }
    case SCENE_PROGRAM: {
      render_program_scene(treadmill);
      break;
    }
    case SCENE_COUNTDOWN: {
      // sd_set_visible(&sd_program, false);
      sd_render_all_visible(false);
      render_training_data(treadmill);
      render_training_data_program(treadmill);
      break;
    }
    case SCENE_TRAINING: {
      sd_set_visible(&sd_program, false);
      render_training_data(treadmill);
      render_training_data_program(treadmill);
      break;
    }
    case SCENE_PAUSED: {
      sd_set_visible(&sd_program, false);
      render_training_data_program(treadmill);
      break;
    }
    case SCENE_REPORT: {
      sd_set_visible(&sd_program, false);
      render_training_data(treadmill);
      break;
    }
    case SCENE_TEST: {
      sd_set_value_to_all(8888); ///led test
      break;
    }
    default:;
  }
}
