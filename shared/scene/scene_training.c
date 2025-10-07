#include "../treadmill_motor_modbus.h"
#include "../treadmill_programs_update.h"
#include "scene.h"
#include "sd/sd_render.h" /////////

// New includes for BLE commands and helpers
#include "../shared/ble/ble_protocol/ble_protocol_command.h"
#include "../shared/ble/ble.h"
#include <string.h>

uint8_t speed_changed = 0;
uint8_t incline_changed = 0;

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  treadmill_clear_bpm_hg_loss(treadmill); // avoid colision with programs fat burn and super cardio
  treadmill->hw.handgrip.state = HEARTRATE_HG_DISABLED; // avoid colision with programs fat burn and super cardio
  switch (key) {
    case KEY_EXT_KBD_STOP:
    case KEY_STOP:
      treadmill_pause_training(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      speed_changed = 1;
      if (treadmill->program.id == PROGRAM_WALK_30MIN) {
        if (treadmill->program.p_walk_30min.target_speed + treadmill->config.speed.step <=
            treadmill->config.speed.max) {
          treadmill->program.p_walk_30min.target_speed += treadmill->config.speed.step;
        }
      }
      if (treadmill->program.id == PROGRAM_WALK_08MIN) { //////////// new program
        if (treadmill->program.p_walk_08min.target_speed + treadmill->config.speed.step <=
            treadmill->config.speed.max) {
          treadmill->program.p_walk_08min.target_speed += treadmill->config.speed.step;
        }
      }
      else {
        treadmill_speed_increment(treadmill);
      }
      break;
    case KEY_EXT_KBD_SPEED_INC: {
        speed_changed = 1;
        int speed = (int)treadmill->training.speed + 100;
        if (speed > treadmill->config.speed.max)
            speed = treadmill->config.speed.max;
        treadmill_set_speed(treadmill, (uint16_t)speed);        						
        break;
    }    
    case KEY_SPEED_DECREMENT:
      speed_changed = 1;
      if (treadmill->program.id == PROGRAM_WALK_30MIN) {
        if ((int)treadmill->program.p_walk_30min.target_speed - treadmill->config.speed.step >=
            treadmill->config.speed.min) {
          treadmill->program.p_walk_30min.target_speed -= treadmill->config.speed.step;
        }
      } 
      if (treadmill->program.id == PROGRAM_WALK_08MIN) { /////////// new program
        if ((int)treadmill->program.p_walk_08min.target_speed - treadmill->config.speed.step >=
            treadmill->config.speed.min) {
          treadmill->program.p_walk_08min.target_speed -= treadmill->config.speed.step;
        }
      }      
      else {
        treadmill_speed_decrement(treadmill);
      }
      break;

    case KEY_EXT_KBD_SPEED_DEC: {
        speed_changed = 1;
        int speed = (int)treadmill->training.speed - 100;
        if (speed < treadmill->config.speed.min)
            speed = treadmill->config.speed.min;
        treadmill_set_speed(treadmill, (uint16_t)speed);						
        break;
    }
    case KEY_INCLINE_INCREMENT:
      incline_changed = 1;
      treadmill_incline_increment(treadmill);
      break;
    case KEY_INCLINE_DECREMENT:
      incline_changed = 1;
      treadmill_incline_decrement(treadmill);
      break;
    case KEY_EXT_KBD_SPEED_6:
      // treadmill_set_bpm_value(treadmill, 40); //debug
      speed_changed = 1;
      treadmill_set_speed(treadmill, 600);
      break;
    case KEY_EXT_KBD_SPEED_9:
      // treadmill_set_bpm_value(treadmill, treadmill->training.bpm - 10); //debug
      speed_changed = 1;
      treadmill_set_speed(treadmill, 900);
      break;
    case KEY_EXT_KBD_SPEED_12:
      // treadmill_set_bpm_value(treadmill, treadmill->training.bpm + 10); //debug
      speed_changed = 1;
      treadmill_set_speed(treadmill, 1200);
      break;
    case KEY_EXT_KBD_SPEED_15:
      speed_changed = 1;
      treadmill_set_speed(treadmill, 1500);
      break;
    case KEY_INCLINE_3:
      break;
    case KEY_INCLINE_6:
      break;
    case KEY_INCLINE_9:
      break;
    case KEY_INCLINE_12:
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
  if (speed_mod >= 5) 
    speed = speed + (10 - speed_mod);
  return speed / 10;
}

// calculate minimum time to change maximum variables speed pace bpm
uint32_t max_var_tick = 0;

static void update(treadmill_t *treadmill) {

  if (treadmill->ms - treadmill->training.ms >= 1000) {
    int32_t ms_delta = (int32_t)1000 - (treadmill->ms - treadmill->training.ms);		
	  treadmill->training.ms = treadmill->ms + ms_delta;
    treadmill->training.seconds++;
    treadmill->training.remaining_seconds--;

    uint32_t mm = (treadmill->training.speed * 27778) / 10000;
    uint32_t cm = mm / 10;
    uint32_t m = cm / 100;
    (void)m;
    treadmill->training.cm += cm;
    treadmill->training.meters = treadmill->training.cm / 100;

    if (treadmill->training.cm % 100 >= 50) { //Lucas added from tft
      treadmill->training.meters++;
    }

    treadmill->training.pace = treadmill_calculate_pace(treadmill->training.speed); 
    treadmill->training.pace_total_seconds += treadmill->training.pace;

    //Lucas 25/02
    if (treadmill->hw.ble.gk.connected) {        	
      treadmill->training.kcal_gk = treadmill->hw.ble.gk.total_energy*1000;                        
      treadmill->training.kcal = treadmill->training.kcal_gk;
    } else {            
      treadmill->training.kcal += treadmill_calculate_calories(treadmill);
    }

    // treadmill->training.kcal += treadmill_calculate_calories(treadmill); //Lucas 25/02

    if (treadmill->hw.ble.connected && treadmill->hw.ble.hrm.bpm <= 0 && treadmill->hw.ble.gk.connected == 0 && treadmill->hw.ble.sh.connected == 0) {
      treadmill->hw.ble.hrm.bpm = 0;
      // do nothing
    } 
    else if (treadmill->hw.ble.connected && treadmill->hw.ble.hrm.bpm > 0){
      treadmill->training.bpm = treadmill->hw.ble.hrm.bpm;
    } // Lucas - 23/03

    else if (treadmill->hw.ble.gk.connected){
      treadmill->training.bpm = (uint8_t)treadmill->hw.ble.gk.bpm;
    }
    else if (treadmill->hw.ble.sh.connected || (treadmill->hw.ble.hrm.bpm > 0)){
      treadmill->training.bpm = treadmill->hw.ble.hrm.bpm;
    }
    else if (treadmill->hw.ble.hrm.status == HRM_STATUS_CONNECTED || (treadmill->hw.ble.hrm.bpm > 0)){
      treadmill->training.bpm = (uint8_t)treadmill->hw.ble.hrm.bpm; // Lucas
    }

    else if ((treadmill->hw.handgrip.state == HEARTRATE_HG_DISABLED) && treadmill->hw.ble.connected == 0){
      treadmill->training.bpm = 0;
    }

    // Auto-reconnect logic for BLE HRM (if module present and HRM lost)
    {
      extern uint8_t ble_allowed;
      static uint32_t hrm_reconnect_tick = 0;
      static uint8_t hrm_reconnect_attempts = 0;
      static uint8_t hrm_connect_sent = 0;
      // Only try when BLE permission is given and HRM is disconnected
      if (ble_allowed && treadmill->hw.ble.hrm.status == HRM_STATUS_DISCONNECTED) {
        uint32_t now = (treadmill->get_time_ms) ? treadmill->get_time_ms() : treadmill->ms;
        if (hrm_reconnect_tick == 0) hrm_reconnect_tick = now;
        // attempt scan every 0.5 seconds
        if ((now - hrm_reconnect_tick) >= 500) {
          hrm_reconnect_tick = now;
          hrm_reconnect_attempts++;
          uint8_t buffer[128];
          uint8_t length = 0;
          // send scan to rediscover straps
          build_hrm_scan_packet(buffer, &length, 128);
          ble_post_message(buffer, "", 0);

          // process received bytes/messages right away (parse scan response if already arrived)
          extern void treadmill_ble_process(treadmill_t *treadmill);
          // treadmill_ble_process(treadmill);

          // if scan populated a device name, and we haven't already requested connect, do it now
          if (treadmill->hw.ble.hrm.device_name[0] != '\0' && treadmill->hw.ble.hrm.status == HRM_STATUS_DISCONNECTED) {
            length = 0;
            build_hrm_connect_packet(buffer, &length, 128);
            treadmill->hw.ble.status = BLE_STATUS_PERMISSION_ALLOWED;
            ble_post_message(buffer, "", 0);

            // process possible immediate responses after connect
            treadmill_ble_process(treadmill);

            hrm_connect_sent = 1;
          }

          // keep attempts bounded (wrap)
          if (hrm_reconnect_attempts >= 12) hrm_reconnect_attempts = 0;
        }
      } else {
        // reset attempts & connect flag when HRM is ok or BLE not available
        hrm_reconnect_tick = 0;
        hrm_reconnect_attempts = 0;
        hrm_connect_sent = 0;
      }
     }
    
    // if ((treadmill->hw.handgrip.state == HEARTRATE_HG_DISABLED) && (treadmill->hw.ble.gk.connected == 0) && (treadmill->hw.ble.hrm.status == HRM_STATUS_DISCONNECTED) && (treadmill->hw.ble.sh.connected == 0)) {
    //   treadmill->training.bpm = 0;
    // }    //Lucas check here

    if (treadmill->training.bpm >= 40 && treadmill->training.bpm <= 240) {
      treadmill->training.bpm_total += treadmill->training.bpm;
      treadmill->training.bpm_count++;
      treadmill->training.bpm_average = treadmill->training.bpm_total / treadmill->training.bpm_count;
      if (treadmill->training.bpm > treadmill->training.bpm_max){
        treadmill->training.bpm_max = treadmill->training.bpm;
      }
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

    // Average speed
    // Atualizar soma e contagem de amostras de velocidade
    treadmill->training.speed_sum += treadmill->training.speed;
    treadmill->training.speed_samples++;
        
    // Calcular velocidade média usando todas as amostras
    if (treadmill->training.speed_samples > 0) {
      treadmill->training.average_speed = 
      treadmill->training.speed_sum / treadmill->training.speed_samples;
    }

    extern void treadmill_ble_send_training_data(treadmill_t *treadmill);
    treadmill_ble_send_training_data(treadmill);
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

  /// 10 seconds mininum is working for max value. 
  /// However, we need to cut max value in calculation of average speed and pace
  // max pace and speed calculation //////////////////////////////////// 
  if ((treadmill->training.pace < treadmill->training.pace_max) && (treadmill->training.pace > 0)){
    // if ((treadmill->training.seconds - max_var_tick) >= (10)){ // 10 seconds minimum
    treadmill->training.pace_max = treadmill->training.pace;
    treadmill->training.speed_max = treadmill->training.speed;
    // max_var_tick = treadmill->training.seconds;
    // }
  }////////////////////////////////////

  // handle program specific things
  treadmill_scene_training_programs_update(treadmill);
}

scene_t scene_training = {.id = SCENE_TRAINING, .name = "TRAINING", .keypress = keypress, .update = update};
