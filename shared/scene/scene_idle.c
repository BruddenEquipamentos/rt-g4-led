#include "../treadmill_motor_modbus.h"
#include "scene.h"
#include "../shared/dm/dm.h"

int run_test = 0;
int test_count = 0;

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  treadmill->idle_tick = treadmill->get_time_ms();
  switch (key) {
    case KEY_START:
    case KEY_EXT_KBD_START:
      treadmill_start_training_countdown(treadmill);
      break;
    case KEY_PROGRAM: { // + key
      if (treadmill->config.modbus) {
        treadmill_set_next_mb_parameter(treadmill);
      } else {
        dm_reset_frame_messages();
        treadmill_program(treadmill);
      }
      break;
    }
    case KEY_INCLINE_6: { // 30 min program key
      dm_reset_frame_messages();
      treadmill->program.id = PROGRAM_WALK_30MIN; // program 5
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_INCLINE_9: { // 200 kcal program key
      dm_reset_frame_messages();
      treadmill->program.id = PROGRAM_200K_CALORIES; // program 8
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_INCLINE_12: { // warmup program key
      dm_reset_frame_messages();
      treadmill->program.id = PROGRAM_WALK_08MIN; // program 9
      program_load_default_value(treadmill);
      treadmill_set_scene(treadmill, SCENE_PROGRAM);
      break;
    }
    case KEY_CONFIG: {
      treadmill_config(treadmill);
      break;
    }
//    case KEY_LUB: {
//      treadmill_set_scene(treadmill, SCENE_LUB);
//      break;
//    }
    // case KEY_SPEED_INCREMENT: {
    //   // how test
    //   if (!run_test) {
    //     run_test = 1;
    //   }
    //   break;
    // }
    // case KEY_SPEED_DECREMENT: {
    //   if (run_test != 1 && run_test != 2) {
    //     test_count = 0;
    //     run_test = 2;
    //   }
    // }
    default:;
  }
}

static void update(treadmill_t *treadmill) {

  // if (treadmill->hw.ble.connected && treadmill->hw.ble.heartrate.bpm > 0) {
  //   treadmill->training.bpm = treadmill->hw.ble.heartrate.bpm;
  // }

  //////////////////// IHRSA
  // if (treadmill->get_time_ms() - treadmill->scene_tick >= 5 * 1000) {
  //   treadmill_start_training_countdown(treadmill);
  //   treadmill->scene_tick = treadmill->get_time_ms();
  // }
  //////////////////// IHRSA

  // if (run_test == 1) {
  //   // erase eeprom
  //   nvm.NVM_Format();
  //   treadmill->total.distance_in_meters = 0;
  //   treadmill->total.time_in_seconds = 0;
  //   run_test = 2;
  //   test_count = 0;

  // } else if (run_test == 2) {
  //   // read and write 5000
  //   treadmill->training.seconds = 300;
  //   treadmill->training.meters = 1000;
  //   treadmill->total.distance_in_meters += treadmill->training.meters;
  //   treadmill->total.time_in_seconds += treadmill->training.seconds;
  //   treadmill_save_total_distance(treadmill, treadmill->total.distance_in_meters);
  //   treadmill_save_total_time(treadmill, treadmill->total.time_in_seconds);    

  //   test_count++;

  //   if (test_count == 10000) {
  //     run_test = 3;
  //   }
  // }

}

const scene_t scene_idle = {.id = SCENE_IDLE, .name = "IDLE", .keypress = keypress, .update = update};
