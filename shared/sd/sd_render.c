#include "sd_render.h"

#include "../panel/render_program_scene.h"
#include "../panel/rt_led.h"

sd_t sd_distance = {.flag = SD_FLAG_VISIBLE};
sd_t sd_time = {.flag = SD_FLAG_VISIBLE};
sd_t sd_speed = {.flag = SD_FLAG_VISIBLE};
sd_t sd_kcal = {.flag = SD_FLAG_VISIBLE};
sd_t sd_program = {.flag = SD_FLAG_VISIBLE};
sd_t sd_pace = {.flag = SD_FLAG_VISIBLE};
sd_t sd_incline = {.flag = SD_FLAG_VISIBLE};
sd_t sd_bpm = {.flag = SD_FLAG_VISIBLE};

void set_all_sd_visible(bool visible) {
  sd_set_visible(&sd_distance, visible);
  sd_set_visible(&sd_time, visible);
  sd_set_visible(&sd_speed, visible);
  sd_set_visible(&sd_kcal, visible);
  sd_set_visible(&sd_program, visible);
  sd_set_visible(&sd_pace, visible);
  sd_set_visible(&sd_incline, visible);
  sd_set_visible(&sd_bpm, visible);
}

static void render_training_data(treadmill_t *treadmill) {
  sd_set_km_m(&sd_distance, treadmill->training.meters);
  sd_set_min_sec(&sd_time, treadmill->training.seconds);
  sd_set_kmh(&sd_speed, treadmill->training.speed);
  sd_set_min_sec(&sd_pace, treadmill->training.pace);

  sd_set_value(&sd_kcal, treadmill->training.kcal / 1000);
  sd_set_value(&sd_incline, treadmill->training.incline);
  sd_set_value(&sd_bpm, treadmill->training.bpm);

  sd_set_value(&sd_program, 0);
}

static void render_training_data_program(treadmill_t *treadmill) {
  switch (treadmill->program.id) {
    case PROGRAM_TIME: {
      uint32_t remaining_seconds =
          treadmill->program.p_time.seconds - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_DISTANCE: {
      uint32_t remaining_meters =
          treadmill->program.p_distance.meters - treadmill->training.meters;
      sd_set_km_m(&sd_distance, remaining_meters);
      break;
    }
    case PROGRAM_CALORIES: {
      uint32_t remaining_kcal =
          treadmill->program.p_calories.kcal - treadmill->training.kcal;
      sd_set_value(&sd_kcal, remaining_kcal);
      break;
    }
    case PROGRAM_HIIT: {
      uint32_t remaining_seconds =
          treadmill->program.p_hiit.seconds - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_WALK_30MIN: {
      uint32_t remaining_seconds = (30 * 60) - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_FAT_BURN: {
      uint32_t remaining_seconds =
          treadmill->program.p_fat_burn.seconds - treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    case PROGRAM_SUPER_CARDIO: {
      uint32_t remaining_seconds = treadmill->program.p_super_cardio.seconds -
                                   treadmill->training.seconds;
      sd_set_min_sec(&sd_time, remaining_seconds);
      break;
    }
    default:
      render_training_data(treadmill);
  }
}

static void render_config_data(treadmill_t *treadmill) {
  set_all_sd_visible(false);
  // options
  switch (treadmill->config.option) {
    case CONFIG_OPTION_MODEL: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.model);
      break;
    }
    case CONFIG_OPTION_LANGUAGE: {
      sd_set_visible(&sd_pace, false);
      break;
    }
    case CONFIG_OPTION_UNIT: {
      sd_set_visible(&sd_pace, false);
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

    case CONFIG_OPTION_LUB: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->config.lub_distance);
      break;
    }

    case CONFIG_OPTION_VER: {
      sd_set_visible(&sd_pace, true);
      sd_set_version(&sd_pace, MAJOR, MINOR, REVISION);
      break;
    }
    case CONFIG_OPTION_TOTAL_DISTANCE: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->total.distance_in_meters / 1000);
      break;
    }
    case CONFIG_OPTION_TOTAL_HOURS: {
      sd_set_visible(&sd_pace, true);
      sd_set_value(&sd_pace, treadmill->total.time_in_seconds / 3600);
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
    default:
      sd_set_visible(&sd_pace, false);
  }
}

void sd_render(treadmill_t *treadmill) {
  set_all_sd_visible(true);
  switch (treadmill->scene) {
    case SCENE_INIT: {
      sd_set_visible(&sd_program, false);
      render_training_data(treadmill);
      break;
    }
    case SCENE_IDLE: {
      sd_set_visible(&sd_program, false);
      render_training_data(treadmill);
      break;
    }
    case SCENE_LUB: {
      sd_set_visible(&sd_program, false);
      render_training_data(treadmill);
      break;
    }
    case SCENE_CONFIG: {
      sd_set_visible(&sd_program, false);
      // render_training_data(treadmill);
      render_config_data(treadmill);
      break;
    }
    case SCENE_PROGRAM: {
      render_program_scene(treadmill);
      break;
    }
    case SCENE_COUNTDOWN: {
      sd_set_visible(&sd_program, false);
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
      render_training_data(treadmill);
      break;
    }
    case SCENE_REPORT: {
      sd_set_visible(&sd_program, false);
      render_training_data(treadmill);
      break;
    }
    default:;
  }
}
