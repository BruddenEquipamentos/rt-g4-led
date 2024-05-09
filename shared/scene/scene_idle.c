#include "../treadmill_motor_modbus.h"
#include "scene.h"
#include "../rt_led/Core/Src/nvm_emb.h"

int run_test = 0;
int test_count = 0;

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  treadmill->idle_tick = treadmill->get_time_ms();
  switch (key) {
    case KEY_START:
      treadmill_start_training_countdown(treadmill);
      break;
    case KEY_PROGRAM: {
      if (treadmill->config.modbus) {
        treadmill_set_next_mb_parameter(treadmill);
      } else {
        treadmill_program(treadmill);
      }
      break;
    }
    case KEY_CONFIG: {
      treadmill_config(treadmill);
      break;
    }
    case KEY_LUB: {
      treadmill_set_scene(treadmill, SCENE_LUB);
      break;
    }
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
