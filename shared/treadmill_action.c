#include "treadmill_action.h"

#include <string.h>

#include "nvm/nvm.h"
#include "treadmill_programs.h"
#include "treadmill_programs_update.h"
#include "ble/ble_protocol/ble_ftms_treadmill.h"
#include "ble/ble_protocol/ble_protocol_helper.h"

#include "../shared/dm/dm.h"
extern int completed_goal;

int report_state = 0; // alternate between avg and max value on report
int incline_limit_reached = 0; ///////////// if user tries to change incline outside boundaries

/////////////////////////////////////////////////////
extern struct interval_t hiit_intervals;
extern uint16_t on_interval;
/////////////////////////////////////////////////////

void treadmill_start_training_countdown(treadmill_t *treadmill) {
  completed_goal = 0;
  treadmill->countdown.value = START_TRAINING_COUNTDOWN_VALUE;
  treadmill->countdown.ms = 0;
  treadmill_set_scene(treadmill, SCENE_COUNTDOWN);
}

void treadmill_start_training(treadmill_t *treadmill) {
  memset(&treadmill->training, 0x00, sizeof(struct training));
  treadmill->training.ms = treadmill->ms;

  //////////////////// max pace max speed max bpm
  treadmill->training.bpm_max = 0;
  treadmill->training.speed_max = 0;
  treadmill->training.pace_max = 5000;

  //////////////////// max pace max speed max bpm
  treadmill_set_speed(treadmill, treadmill->config.speed.min);  //Lucas --- no tft10 não tem

  treadmill_set_scene(treadmill, SCENE_TRAINING);
  if (treadmill->on_update_fms_callback) {
    treadmill->on_update_fms_callback(treadmill, StartedOrResumedByTheUser, NULL);
  }  
}

void treadmill_abort_countdown(treadmill_t *treadmill) { 
  treadmill_exit_program(treadmill); ///////////////
  treadmill_set_scene(treadmill, SCENE_IDLE); 
}

void treadmill_pause_training(treadmill_t *treadmill) {
  treadmill_clear_dm_message(treadmill);  
  treadmill_set_scene(treadmill, SCENE_PAUSED);
  if (treadmill->on_update_fms_callback) {
    uint8_t paused = 0x02;
    treadmill->on_update_fms_callback(treadmill, StoppedOrPausedByTheUser, &paused);
  }
  treadmill_set_speed(treadmill, 0);
}

void treadmill_reset_training(treadmill_t *treadmill) {
  memset(&treadmill->training, 0x00, sizeof(struct training));
  memset(&treadmill->aux, 0x00, sizeof(struct aux));
  memset(&treadmill->program, 0x00, sizeof(struct program));
  memset(&hiit_intervals, 0x00, sizeof(interval_t) * (PROGRAM_HIIT_MAX_SERIES * 2));  /////// reset hiit invervals
  memset(&on_interval, 0x00, sizeof(uint16_t)); /////// reset hiit invervals
  treadmill_clear_bpm_hg_loss(treadmill); //////////////
  treadmill_set_bpm_stop(treadmill, 0); /////////////////
  if (treadmill->on_update_fms_callback){
	  treadmill->on_update_fms_callback(treadmill, Reset, NULL);
  }
  // Zerar as novas variáveis de velocidade média
  treadmill->training.speed_sum = 0;
  treadmill->training.speed_samples = 0;
}

void treadmill_resume_training(treadmill_t *treadmill) {  
  treadmill->training.ms = treadmill->ms;
  treadmill_set_scene(treadmill, SCENE_TRAINING);
  ////////////////// return to 1.2 km/h
  if (treadmill->program.id == PROGRAM_WALK_08MIN){
    treadmill->program.p_walk_08min.target_speed = (int)treadmill->config.speed.min;
  }
  ////////////////// return to 1.2 km/h
  treadmill_set_speed(treadmill, treadmill->config.speed.min); //Lucas --- no tft10 não tem

  if (treadmill->on_update_fms_callback) {
    treadmill->on_update_fms_callback(treadmill, StartedOrResumedByTheUser, NULL);
  }
}

void treadmill_finish_training(treadmill_t *treadmill) {
  treadmill_clear_dm_message(treadmill);
  treadmill_set_speed(treadmill, 0);
  treadmill_set_scene(treadmill, SCENE_REPORT);
  report_state = 0; ///////////////////////////
  memset(&treadmill->hw.ble.hrm, 0, sizeof(treadmill->hw.ble.hrm)); //Lucas -- disconnect from BLE strap only
  if (treadmill->on_update_fms_callback) {
    uint8_t stopped = 0x01;
    treadmill->on_update_fms_callback(treadmill, StoppedOrPausedByTheUser, &stopped);
  }
}

void treadmill_finish_program(treadmill_t *treadmill) { 
  dm_reset_frame();
  treadmill_finish_training(treadmill); 
}

static void treadmill_save_training(treadmill_t *treadmill) {
  treadmill->total.distance_in_meters += treadmill->training.meters;
  treadmill->total.time_in_seconds += treadmill->training.seconds;
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS, treadmill->total.distance_in_meters);
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_TIME_SECONDS, treadmill->total.time_in_seconds);
}

void treadmill_close_report(treadmill_t *treadmill) {
  treadmill_save_training(treadmill);
  treadmill_reset_training(treadmill);
  treadmill->program.id = PROGRAM_NONE;
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

void treadmill_speed_increment(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed + treadmill->config.speed.step <= treadmill->config.speed.max) {
    treadmill_set_speed(treadmill, treadmill->training.speed + treadmill->config.speed.step);
  }
}

void treadmill_speed_decrement(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed - treadmill->config.speed.step >= treadmill->config.speed.min) {
    treadmill_set_speed(treadmill, treadmill->training.speed - treadmill->config.speed.step);
  }
}

int treadmill_set_speed(treadmill_t *treadmill, const uint16_t speed) {
  if (speed > treadmill->config.speed.max) return 0;
  if (speed > 0 && speed < treadmill->config.speed.min) return 0;
  treadmill->training.speed = speed;
  treadmill->training.pace = treadmill_calculate_pace(treadmill->training.speed);
  treadmill->aux.speed_target = speed;

  // todo: refactor ///////////////////////////// hiit refactor
  if (treadmill->training.seconds > 0 && speed > 0) {
    if (treadmill->program.id == PROGRAM_HIIT) {
      if ((on_interval == 0) || (on_interval % 2 == 0)){
        treadmill->program.p_hiit.speed_t1 = speed;
      }
      else{
        treadmill->program.p_hiit.speed_t2 = speed;
      }
    }
  }

  // if (treadmill->on_update_fms_callback) { //Lucas --- no tft10 não tem
  //   treadmill->on_update_fms_callback(treadmill, TargetSpeedChanged, &speed);
  // }
  return 1;
}

int treadmill_set_incline(treadmill_t *treadmill, const int16_t incline) {
  if (incline > treadmill->config.incline.max) return 0;
  if (incline < treadmill->config.incline.min) return 0;
  if (treadmill->training.incline != incline) {
    treadmill->aux.incline_changed_time_ms = treadmill->get_time_ms();
    treadmill->training.incline = incline;

    // if (treadmill->on_update_fms_callback) { //Lucas --- no tft10 não tem
    //   treadmill->on_update_fms_callback(treadmill, TargetInclineChanged, &incline);
    // }
  }
  return 1;
}

void treadmill_incline_increment(treadmill_t *treadmill) {
  if (treadmill->training.incline >= treadmill->config.incline.min &&
      treadmill->training.incline + treadmill->config.incline.step <= treadmill->config.incline.max) {
    treadmill_set_incline(treadmill, treadmill->training.incline + treadmill->config.incline.step);
  }
}

void treadmill_incline_decrement(treadmill_t *treadmill) {
  if (treadmill->training.incline >= treadmill->config.incline.min &&
      treadmill->training.incline - treadmill->config.incline.step >= treadmill->config.incline.min) {
    treadmill_set_incline(treadmill, treadmill->training.incline - treadmill->config.incline.step);
  }
}

void treadmill_program(treadmill_t *treadmill) {
  treadmill->program.id = 1;
  program_load_default_value(treadmill);
  treadmill_set_scene(treadmill, SCENE_PROGRAM);
}

void treadmill_next_program(treadmill_t *treadmill) {
  if (treadmill->program.id + 1 >= PROGRAM_TOTALS)
    treadmill->program.id = 1;
  else{
    treadmill->program.id++;
  }
  dm_reset_frame_messages();
  treadmill->program.param_index = 0;
  program_load_default_value(treadmill);
}

void treadmill_exit_program(treadmill_t *treadmill) {
  memset(&treadmill->program, 0x00, sizeof(struct program));
  treadmill_set_scene(treadmill, SCENE_IDLE);
  dm_reset_frame_messages(); /////////////
}

void treadmill_start_program(treadmill_t *treadmill) {
  //
  treadmill_start_training_countdown(treadmill);
}

void treadmill_config(treadmill_t *treadmill) {
  treadmill->config.option = 0;
  memcpy(&treadmill->previous_config, &treadmill->config, sizeof(struct config));
  treadmill_set_scene(treadmill, SCENE_CONFIG);
}

void treadmill_exit_config(treadmill_t *treadmill) {
  treadmill_save_config(treadmill);
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

static int valid_ns(const char *nsu, const int length) {
  if (length > 22) return 0;
  return 1;
}

int treadmill_save_nsu(treadmill_t *treadmill, const char *nsu, const int length) {
  if (!valid_ns(nsu, length)) return 0;
  strcpy(treadmill->config.nsu, nsu);
  nvm.NVM_WriteString(NVM_NSU_START_ADDRESS, nsu, length, NVM_NS_MAX_LENGTH);
  return 1;
}

int treadmill_save_nsp(treadmill_t *treadmill, const char *nsp, const int length) {
  if (!valid_ns(nsp, length)) return 0;
  strcpy(treadmill->config.nsp, nsp);
  nvm.NVM_WriteString(NVM_NSP_START_ADDRESS, nsp, length, NVM_NS_MAX_LENGTH);
  return 1;
}

int treadmill_save_nsi(treadmill_t *treadmill, const char *nsi, const int length) {
  if (!valid_ns(nsi, length)) return 0;
  strcpy(treadmill->config.nsi, nsi);
  nvm.NVM_WriteString(NVM_NSI_START_ADDRESS, nsi, length, NVM_NS_MAX_LENGTH);
  return 1;
}

int treadmill_save_total_distance(treadmill_t *treadmill, uint32_t total_distance_in_meters) {
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS, total_distance_in_meters);
  return 1;
}

int treadmill_save_total_time(treadmill_t *treadmill, uint32_t total_time_in_seconds) {
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_TIME_SECONDS, total_time_in_seconds);
  return 1;
}

int treadmill_start_or_resume(treadmill_t *treadmill) {
  // todo: implementation
  if (treadmill->scene == SCENE_IDLE) {
    treadmill_start_training_countdown(treadmill);
  } else if (treadmill->scene == SCENE_PAUSED) {
    treadmill_resume_training(treadmill);
  }
  return 1;
}

int treadmill_stop_procedure(treadmill_t *treadmill) {
  treadmill_finish_training(treadmill);
  treadmill_close_report(treadmill);
  return 1;
}

int treadmill_pause_procedure(treadmill_t *treadmill) {
  treadmill_pause_training(treadmill);
  return 1;
}

static void build_ftms_data_packet(treadmill_t *treadmill, uint8_t *buffer, uint8_t max_size, uint8_t *size, uint16_t flags) {
    uint16_t instantaneous_speed = treadmill->training.speed;
    uint16_t average_speed = treadmill->training.average_speed; //Lucas
    uint32_t total_distance = treadmill->training.meters;
    uint16_t elapsed_time = treadmill->training.seconds;
    uint16_t inclination = treadmill->training.incline;
    uint16_t calories = treadmill->training.kcal / 1000;

    buffer[(*size)++] = BLE_PROTOCOL_MESSAGE_START;
    flat_bin_to_ascii_8(buffer, size, max_size, BP_NOTIFY);
    flat_bin_to_ascii_16(buffer, size, max_size, FTMS_TREADMILL_DATA_UUID);

    flat_bin_to_ascii_16(buffer, size, max_size, flags);

    if ((flags & TD_MORE_DATA) == 0) {
        flat_bin_to_ascii_16(buffer, size, max_size, instantaneous_speed);
    }

    if (flags & TD_AVERAGE_SPEED_PRESENT) { //Lucas
        flat_bin_to_ascii_16(buffer, size, max_size, average_speed);
    } 

    if (flags & TD_TOTAL_DISTANCE_PRESENT) {
        flat_bin_to_ascii_24(buffer, size, max_size, total_distance);
    }

    if (flags & TD_INCLINATION_RAMP_ANGLE_SETTING_PRESENT) {
        flat_bin_to_ascii_16(buffer,size, max_size, inclination);
        flat_bin_to_ascii_16(buffer, size, max_size, 0x7fff);
    }

    if (flags & TD_ELEVATION_GAIN_PRESENT) {
        // flat_bin_to_ascii_16(buffer, size, max_size, 0xffff);
        // flat_bin_to_ascii_16(buffer, size, max_size, 0xffff);
        flat_bin_to_ascii_16(buffer, size, max_size, 0);
        flat_bin_to_ascii_16(buffer, size, max_size, 0);
    }

    if (flags & TD_EXPENDED_ENERGY_PRESENT) {
        flat_bin_to_ascii_16(buffer, size, max_size, calories); // total energy kcal
        flat_bin_to_ascii_16(buffer, size, max_size, 0xffff); // energy per hour kcal
        flat_bin_to_ascii_8(buffer, size, max_size, 0xff);  // energy per minute kcal
    }

    if (flags & TD_HEART_RATE_PRESENT) {
        flat_bin_to_ascii_8(buffer, size, max_size, 0);
    }

    if (flags & TD_ELAPSED_TIME_PRESENT) {
        flat_bin_to_ascii_16(buffer, size, max_size, elapsed_time);
    }

    uint8_t cs = ble_protocol_checksum(buffer, *size);
    flat_bin_to_ascii_8(buffer, size, max_size, cs);
    buffer[(*size)++] = BLE_PROTOCOL_MESSAGE_END;
}

// TODO: Refactor place
#include "ble/ble_protocol/ble_ftms_treadmill.h"
#include "ble/ble_protocol/ble_protocol_helper.h"
int treadmill_send_ftms_data(treadmill_t *treadmill) {
#define BUFSIZE 128
  uint8_t buffer[BUFSIZE] = {0};
  uint8_t bindex = 0;

  uint16_t instantaneous_speed = treadmill->training.speed;
  uint16_t average_speed = treadmill->training.average_speed; //Lucas
  uint32_t total_distance = treadmill->training.meters;
  uint16_t elapsed_time = treadmill->training.seconds;
  uint16_t inclination = treadmill->training.incline;
  uint16_t calories = treadmill->training.kcal / 1000;    

  // packet size = 2 + 2 + 3 + 4 + 4 + 4 + 1 + 1 + 2 = 23 bytes    
  const packet_size = 25;    
  const mtu = treadmill->hw.ble.connection.mtu > 0 ? treadmill->hw.ble.connection.mtu - 3 : 0;
  
  if (mtu && mtu < packet_size) {
      // split in two
      uint16_t flags = 0x0000;
      bindex = 0;
      flags |= TD_MORE_DATA;             // -ok
      flags |= TD_ELAPSED_TIME_PRESENT;  // -ok      
      flags |= TD_AVERAGE_SPEED_PRESENT; // -ok
      build_ftms_data_packet(treadmill, buffer, BUFSIZE, &bindex, flags);
      // send packet
      treadmill->hw.ble.send_data(buffer, bindex);

      flags = 0x0000;
      bindex = 0;
      flags |= TD_TOTAL_DISTANCE_PRESENT; // -ok
      flags |= TD_INCLINATION_RAMP_ANGLE_SETTING_PRESENT; // -ok
      flags |= TD_ELEVATION_GAIN_PRESENT; // -ok
      flags |= TD_EXPENDED_ENERGY_PRESENT; // -ok
      build_ftms_data_packet(treadmill, buffer, BUFSIZE, &bindex, flags);
  } else {
      // single packet
      uint16_t flags = 0x0000;
      bindex = 0;
      flags |= TD_AVERAGE_SPEED_PRESENT; // -ok
      flags |= TD_TOTAL_DISTANCE_PRESENT; // -ok
      flags |= TD_INCLINATION_RAMP_ANGLE_SETTING_PRESENT; // -ok
      flags |= TD_ELEVATION_GAIN_PRESENT; // -ok
      flags |= TD_EXPENDED_ENERGY_PRESENT; // -ok
      // // flags |= TD_HEART_RATE_PRESENT;
      flags |= TD_ELAPSED_TIME_PRESENT; // -ok
      build_ftms_data_packet(treadmill, buffer, BUFSIZE, &bindex, flags);        
  }

  return treadmill->hw.ble.send_data(buffer, bindex);
}
void treadmill_clear_bpm_hg_loss(treadmill_t *treadmill) {
  treadmill->hg_lost_tick = 0; ///////
  treadmill->hg_lost_count = 0; ////////
}

void treadmill_set_bpm_read(treadmill_t *treadmill) { 
  if (treadmill->hw.ble.gk.connected) { 
      return;
  }
  if (treadmill->hw.ble.sh.connected) {
      return;
  }
  if (treadmill->hw.ble.hrm.status == HRM_STATUS_CONNECTED) { 
    return;
  }
  if (treadmill->hw.ble.ftms.subscribed && treadmill->hw.ble.hrm.bpm > 0) {
    return;
  }
  treadmill->hw.handgrip.state = HEARTRATE_HG_INITING;
  treadmill_clear_bpm_hg_loss(treadmill); /////
}

void treadmill_set_bpm_value(treadmill_t *treadmill, uint8_t bpm) {

  if (treadmill->hw.ble.gk.connected) {
    return;
  }
  if (treadmill->hw.ble.sh.connected) {
    return;
  }
  if (treadmill->hw.ble.hrm.status == HRM_STATUS_CONNECTED) { 
    return;
  }
  if (treadmill->hw.ble.ftms.subscribed && treadmill->hw.ble.hrm.bpm > 0) {
    return;
  }

  treadmill->hw.handgrip.state = HEARTRATE_HG_READING;
  treadmill->training.bpm = bpm;
  if (treadmill->training.bpm >= treadmill->training.bpm_max){ //////////////////// bpm max
    treadmill->training.bpm_max = treadmill->training.bpm;
  }
}

void treadmill_set_bpm_stop(treadmill_t *treadmill, uint8_t in_program) {
  switch (in_program) {
    case 0:{
      treadmill->hw.handgrip.state = HEARTRATE_HG_DISABLED;
      treadmill_clear_bpm_hg_loss(treadmill);
      break;
    }
    case 1: {
      treadmill->hw.handgrip.state = HEARTRATE_HG_LOST;
      treadmill->hg_lost_tick = treadmill->get_time_ms();
      break;
    }
    default: {
      treadmill->hw.handgrip.state = HEARTRATE_HG_DISABLED;
      treadmill_clear_bpm_hg_loss(treadmill);
      break;
    }
  } 
  // if (treadmill->hw.ble.hrm.bpm <= 0) {
  //   treadmill->training.bpm = 0;
  // }

}
