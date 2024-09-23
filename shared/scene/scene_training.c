#include "../treadmill_motor_modbus.h"
#include "../treadmill_programs_update.h"
#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  treadmill_clear_bpm_hg_loss(treadmill); // avoid colision with programs fat burn and super cardio
  treadmill->hw.handgrip.state = HEARTRATE_HG_DISABLED; // avoid colision with programs fat burn and super cardio
  switch (key) {
    case KEY_STOP:
      treadmill_pause_training(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      if (treadmill->program.id == PROGRAM_WALK_30MIN) {
        if (treadmill->program.p_walk_30min.target_speed + treadmill->config.speed.step <=
            treadmill->config.speed.max) {
          treadmill->program.p_walk_30min.target_speed += treadmill->config.speed.step;
        }
      } else {
        treadmill_speed_increment(treadmill);
      }
      break;
    case KEY_SPEED_DECREMENT:
      if (treadmill->program.id == PROGRAM_WALK_30MIN) {
        if ((int)treadmill->program.p_walk_30min.target_speed - treadmill->config.speed.step >=
            treadmill->config.speed.min) {
          treadmill->program.p_walk_30min.target_speed -= treadmill->config.speed.step;
        }
      } else {
        treadmill_speed_decrement(treadmill);
      }
      break;
    case KEY_INCLINE_INCREMENT:
      treadmill_incline_increment(treadmill);
      break;
    case KEY_INCLINE_DECREMENT:
      treadmill_incline_decrement(treadmill);
      break;
    case KEY_SPEED_6:
      // treadmill_set_bpm_value(treadmill, 40);
      treadmill_set_speed(treadmill, 600);
      break;
    case KEY_SPEED_9:
      // treadmill_set_bpm_value(treadmill, treadmill->training.bpm - 10);
      treadmill_set_speed(treadmill, 900);
      break;
    case KEY_SPEED_12:
      // treadmill_set_bpm_value(treadmill, treadmill->training.bpm + 10);
      treadmill_set_speed(treadmill, 1200);
      break;
    case KEY_SPEED_15:
      treadmill_set_speed(treadmill, 1500);
      break;
    case KEY_INCLINE_3:
      treadmill_set_incline(treadmill, 30);
      break;
    case KEY_INCLINE_6:
      treadmill_set_incline(treadmill, 60);
      break;
    case KEY_INCLINE_9:
      treadmill_set_incline(treadmill, 90);
      break;
    case KEY_INCLINE_12:
      treadmill_set_incline(treadmill, 120);
      break;
    case KEY_PROGRAM: {
      if (treadmill->config.modbus) {
        treadmill_set_next_mb_parameter(treadmill);
      }
      break;
    }
    default:;
  }
}

// CHECK HERE
static uint32_t treadmill_calculate_speed(uint32_t dm, uint32_t seconds) {
  volatile uint32_t cm_sec = (dm * 100 / seconds);
  uint32_t speed = ((cm_sec * 36) / 1000);  // 333 * 36 = 1198
  uint32_t speed_mod = (speed) % 10;
  if (speed_mod >= 5) speed = speed + (10 - speed_mod);
  return speed / 10;
}

static void update(treadmill_t *treadmill) {
  if (treadmill->ms - treadmill->training.ms >= 1000) {
    treadmill->training.ms = treadmill->ms;

    treadmill->training.seconds++;
    treadmill->training.remaining_seconds--;

    uint32_t mm = (treadmill->training.speed * 27778) / 10000;
    uint32_t cm = mm / 10;
    uint32_t m = cm / 100;
    (void)m;
    treadmill->training.cm += cm;
    treadmill->training.meters = treadmill->training.cm / 100;

    treadmill->training.pace = treadmill_calculate_pace(treadmill);
    treadmill->training.kcal += treadmill_calculate_calories(treadmill);

    treadmill->training.pace_total_seconds += treadmill->training.pace;

    if (treadmill->training.bpm >= 40 && treadmill->training.bpm <= 240) {
      treadmill->training.bpm_total += treadmill->training.bpm;
      treadmill->training.bpm_count++;
      treadmill->training.bpm_average = treadmill->training.bpm_total / treadmill->training.bpm_count;
    }

    {
      // Compute dot matrix data to be shown
      static uint32_t mm_per_minute = 0;
      mm_per_minute += mm;

      if (treadmill->training.seconds && treadmill->training.seconds % 60 == 0) {
        uint32_t minute = (treadmill->training.seconds / 60);

        treadmill->aux.average_history[treadmill->aux.average_history_index].minute = minute;
        treadmill->aux.average_history[treadmill->aux.average_history_index].average_speed =
            treadmill_calculate_speed(mm_per_minute, 60);

        if (minute >= 16) {
          for (int i = 1; i < 16; ++i) {
            treadmill->aux.average_history[i - 1] = treadmill->aux.average_history[i];
          }
        } else if (minute < 16) {
          treadmill->aux.average_history_index = (treadmill->aux.average_history_index + 1) % 16;
        }

        mm_per_minute = mm;
      }
    }
  }

  // Simulate time that it takes to reach the targeted speed
  if (treadmill->ms - treadmill->aux.speed_ms >= 100) {
    treadmill->aux.speed_ms = treadmill->ms;
    if (treadmill->aux.speed_reached != treadmill->aux.speed_target) {
      if (treadmill->aux.speed_target > treadmill->aux.speed_reached) {
        treadmill->aux.speed_reached += 10;
      } else if (treadmill->aux.speed_target < treadmill->aux.speed_reached) {
        treadmill->aux.speed_reached -= 10;
      }
    }
  }
  // handle program specific things
  treadmill_scene_training_programs_update(treadmill);
}

scene_t scene_training = {.id = SCENE_TRAINING, .name = "IDLE", .keypress = keypress, .update = update};
