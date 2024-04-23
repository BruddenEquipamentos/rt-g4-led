#include "../treadmill_motor_modbus.h"
#include "scene.h"

uint8_t test_id = 0;
uint8_t test_message_id = 0; 
uint8_t test_message_speed = 50;
uint8_t test_road_speed = 50;

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

    case KEY_SPEED_INCREMENT: {
      if (test_message_speed - 5 >= 5) test_message_speed -= 5;
      break;
    }

    case KEY_SPEED_DECREMENT: {
      if (test_message_speed + 5 <= 500) test_message_speed += 5;      
      break;
    }

    case KEY_SPEED_15: {
      test_id = 0;
      if (test_message_id + 1 < 3) test_message_id++; 
      else test_message_id = 0;
      break;
    }
    case KEY_SPEED_12: {      
      test_id = 1;
      break;
    }
    case KEY_SPEED_9: {
      test_id = 2;
      break;
    }
    
    case KEY_SPEED_6: {
      test_id = 3;
      break;
    }

    case KEY_CONFIG: {
      treadmill_config(treadmill);
      break;
    }
    case KEY_LUB: {
      // treadmill_set_scene(treadmill, SCENE_LUB);
    }
    default:;
  }
}

static void update(treadmill_t *treadmill) {
  if (treadmill_calculate_lub_distance_remaining(treadmill) <= LUB_NOTIFICATION_DISTANCE_REMAINING_KM &&
      treadmill->get_time_ms() - treadmill->idle_tick >= IDLE_SCENE_TIME) {
    // if (!treadmill->config.modbus) treadmill_set_scene(treadmill, SCENE_LUB);
  }
}

const scene_t scene_idle = {.id = SCENE_IDLE, .name = "IDLE", .keypress = keypress, .update = update};
