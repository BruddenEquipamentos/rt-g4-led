#include "treadmill_programs_update.h"

#include "treadmill.h"
#include "treadmill_action.h"
#include "treadmill_programs.h"

#include <stddef.h>

#include "../shared/dm/dm.h"

int completed_goal = 0;

extern uint8_t gk_was_connected;
extern uint8_t gk_lost_bpm;
extern uint8_t ble_strap_was_connected;
extern uint8_t ble_strap_lost_bpm;

typedef struct program_training_callback {
  program_id_t id;
  void (*update)(treadmill_t *treadmill);
} program_training_callback_t;

/////// new update
static void update_program_none(treadmill_t *treadmill) {
  treadmill->training.remaining_seconds = 0; // avoid overflow on not used variable
  if ((treadmill->config.max_time != 0) && treadmill->training.seconds >= (uint32_t)treadmill->config.max_time){
    treadmill_finish_program(treadmill);
  }
}

static void update_program_time(treadmill_t *treadmill) {
  treadmill->training.total_seconds = treadmill->program.p_time.seconds;
  treadmill->training.remaining_seconds = treadmill->training.total_seconds - treadmill->training.seconds;
  treadmill->training.conclusion_percentage = (treadmill->training.seconds * 100 / treadmill->program.p_time.seconds);
  ///////////////////////
  if (treadmill->training.seconds >= treadmill->program.p_time.seconds) {
    completed_goal = 1;
    treadmill_finish_program(treadmill);
  }
}

static void update_program_distance(treadmill_t *treadmill) {
  if (treadmill->training.meters >= treadmill->program.p_distance.meters) {
    completed_goal = 1;
    treadmill_finish_program(treadmill);
  }
}

static void update_program_calories(treadmill_t *treadmill) {
  if ((treadmill->training.kcal / 1000) >= treadmill->program.p_calories.kcal) {
    completed_goal = 1;
    treadmill_finish_program(treadmill);
  }
}

////// new program
static void update_program_200k_calories(treadmill_t *treadmill) {
  if ((treadmill->training.kcal / 1000) >= treadmill->program.p_200k_calories.kcal) {
    completed_goal = 1;
    treadmill_finish_program(treadmill);
  }
}

interval_t hiit_intervals[20] = {0};
uint16_t on_interval = 0;

static void update_program_hiit(treadmill_t *treadmill) {
  treadmill->program.p_hiit.seconds = (treadmill->program.p_hiit.series * treadmill->program.p_hiit.seconds_t1) + (treadmill->program.p_hiit.series * treadmill->program.p_hiit.seconds_t2);
  treadmill->training.total_seconds = treadmill->program.p_hiit.seconds;
  treadmill->training.remaining_seconds = treadmill->training.total_seconds - treadmill->training.seconds;
  
  uint16_t s1 = treadmill->program.p_hiit.speed_t1;                                                                                                 
  uint16_t s2 = treadmill->program.p_hiit.speed_t2;                                                                                                 
  uint32_t t1 = treadmill->program.p_hiit.seconds_t1;
  uint32_t t2 = treadmill->program.p_hiit.seconds_t2;    
                                                                                         
  for (uint16_t i = 0; i < treadmill->program.p_hiit.series * 2; i += 2) {                                        
    hiit_intervals[i].speed = s1;                                                                   
    hiit_intervals[i].seconds = t1 + ((t1+t2) * i/2);                                                
                                                                                                                     
    hiit_intervals[i+1].speed = s2;                                                                                     
    hiit_intervals[i+1].seconds = (t1 + t2) + ((t1+t2) * i/2);
  }
  // test hiit - explosion of time value 
   if (treadmill->program.p_hiit.seconds >= 12000){
     treadmill->program.p_hiit.seconds = 11999;
     hiit_intervals[(treadmill->program.p_hiit.series * 2) - 1].seconds -= 1;
     treadmill->training.remaining_seconds -=  1;
   }
    
  if (treadmill->training.seconds < treadmill->program.p_hiit.seconds){
    if (treadmill->training.seconds >= hiit_intervals[on_interval].seconds){
      on_interval++;
    }
    else{
      treadmill_set_speed(treadmill, hiit_intervals[on_interval].speed);
    }
  }
  else {
    completed_goal = 1;
    treadmill_finish_program(treadmill);
  }
}

static void update_program_walk_30min(treadmill_t *treadmill) {
  treadmill->training.total_seconds = (30 * 60);
  treadmill->training.remaining_seconds = treadmill->training.total_seconds - treadmill->training.seconds;

  if (treadmill->training.seconds < 30 * 60) {
    int calculated_speed = treadmill->program.p_walk_30min.target_speed;

    if (treadmill->training.seconds < 5 * 60)
      calculated_speed -= 200;
    else if (treadmill->training.seconds < 10 * 60){
      calculated_speed -= 100;
    }
    else if (treadmill->training.seconds < 20 * 60) {
    } else if (treadmill->training.seconds < 25 * 60){
      calculated_speed -= 100;
    }
    else if (treadmill->training.seconds < 30 * 60){
      calculated_speed -= 200;
    }

    if (calculated_speed < treadmill->config.speed.min) {
      calculated_speed = treadmill->config.speed.min;
      treadmill_set_speed(treadmill, treadmill->config.speed.min);
    } 
    else {
      treadmill_set_speed(treadmill, calculated_speed);      
    }
  } else {
    completed_goal = 1;
    treadmill_finish_program(treadmill);
  }
}

static int round_speed_update(int num){
  int soma = 0;
  if (num % 10 >= 5){
    soma = 10;
  }
  else{ 
    soma = 0;
  }
  num = num/10;
  num *= 10;
  num += soma;
  return num;
}

/////////////////// new program
static void update_program_walk_08min(treadmill_t *treadmill) {
  treadmill->training.total_seconds = (8 * 60);
  treadmill->training.remaining_seconds = treadmill->training.total_seconds - treadmill->training.seconds;
  
  int v1, v2, v3, v4, v5 = 0;

  if (treadmill->training.seconds < 8 * 60) {

    v1 = (int)treadmill->program.p_walk_08min.target_speed;
    v2 = round_speed_update(v1 * 120/100);
    v3 = round_speed_update(v2 * 83/100);
    v4 = round_speed_update(v3 * 120/100);
    v5 = round_speed_update(v4 * 83/100);

    if (treadmill->training.seconds < 2 * 60){
      treadmill_set_speed(treadmill, v1);
    } 
    else if ((treadmill->training.seconds >= 2 * 60) && (treadmill->training.seconds < 3 * 60)){
      if (v2 < treadmill->config.speed.min) {
        v2 = treadmill->config.speed.min;
        treadmill_set_speed(treadmill, treadmill->config.speed.min);
      } 
      else {
        treadmill_set_speed(treadmill, v2);
      }
    }
    else if ((treadmill->training.seconds >= 3 * 60) && (treadmill->training.seconds < 5 * 60)){
      if (v3 < treadmill->config.speed.min) {
        v3 = treadmill->config.speed.min;
        treadmill_set_speed(treadmill, treadmill->config.speed.min);
      } 
      else {
        treadmill_set_speed(treadmill, v3);
      }
    }
    else if ((treadmill->training.seconds >= 5 * 60) && (treadmill->training.seconds < 6 * 60)){
      if (v4 < treadmill->config.speed.min) {
        v4 = treadmill->config.speed.min;
        treadmill_set_speed(treadmill, treadmill->config.speed.min);
      } 
      else {
        treadmill_set_speed(treadmill, v4);
      }
    }
    else if (treadmill->training.seconds >= 6 * 60){
      if (v5 < treadmill->config.speed.min) {
        v5 = treadmill->config.speed.min;
        treadmill_set_speed(treadmill, treadmill->config.speed.min);
      } 
      else {
        treadmill_set_speed(treadmill, v5);
      }
    }
  } else {
    completed_goal = 1;
    treadmill_finish_program(treadmill);
  }
}

static int treadmill_has_incline(treadmill_t *treadmill) { return treadmill->config.incline.step > 0; }

static int treadmill_can_increment_incline(treadmill_t *treadmill) {
  return (treadmill->config.incline.step > 0) &&
         (treadmill->training.incline + treadmill->config.incline.step <= treadmill->config.incline.max);
}

static int treadmill_can_decrement_incline(treadmill_t *treadmill) {
  return (treadmill->config.incline.step > 0) &&
         (treadmill->training.incline - treadmill->config.incline.step >= treadmill->config.incline.min);
}

static void treadmill_hr_program_below_range_handler(treadmill_t *treadmill) {
  if (treadmill_has_incline(treadmill)) {
    if (treadmill_can_increment_incline(treadmill)) {
      treadmill_incline_increment(treadmill);
    } else {
      // treadmill_set_dm_message(treadmill, "VEL+", 10000, 1000);
    }
  } else {
    // treadmill_set_dm_message(treadmill, "VEL+", 10000, 1000);
  }
  treadmill_set_dm_message(treadmill, "  @+", 4000, 1000);
}

static void treadmill_hr_program_above_range_handler(treadmill_t *treadmill) {
  if (treadmill_has_incline(treadmill)) {
    if (treadmill_can_decrement_incline(treadmill)) {
      treadmill_incline_decrement(treadmill);
    } else {
      // treadmill_set_dm_message(treadmill, "VEL-", 10000, 1000);
    }
  } else {
    // treadmill_set_dm_message(treadmill, "VEL-", 10000, 1000);
  }
  treadmill_set_dm_message(treadmill, "  @-", 4000, 1000);
}

static void update_program_fat_burn(treadmill_t *treadmill) {
  struct p_fat_burn *p_fat_burn = &treadmill->program.p_fat_burn;

  treadmill->training.total_seconds = p_fat_burn->seconds;
  treadmill->training.remaining_seconds = treadmill->training.total_seconds - treadmill->training.seconds;

  switch (p_fat_burn->hr_program_state) {
    case HEART_RATE_PROGRAM_STATE_INIT: {
      p_fat_burn->tick_ms = treadmill->get_time_ms();
      p_fat_burn->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      if (treadmill->hw.ble.gk.connected || treadmill->hw.ble.sh.connected) { //Lucas sh
        gk_was_connected = 1;
        gk_lost_bpm = 0;
		  }
    	if (treadmill->hw.ble.hrm.status == HRM_STATUS_CONNECTED) {
        ble_strap_was_connected = 1;
        ble_strap_lost_bpm = 0;
		  }
      break;
    }
    case HEART_RATE_PROGRAM_STATE_WAIT: {
      if (treadmill->get_time_ms() - p_fat_burn->tick_ms >= 10000) {
        p_fat_burn->hr_program_state = HEART_RATE_PROGRAM_STATE_ACTION;
        p_fat_burn->tick_ms = 0;
      }
      break;
    }
    case HEART_RATE_PROGRAM_STATE_ACTION: {
      if (treadmill->training.bpm && treadmill->training.bpm < p_fat_burn->low_bpm) {
        treadmill_hr_program_below_range_handler(treadmill);
      } else if (treadmill->training.bpm && treadmill->training.bpm > p_fat_burn->high_bpm) {
        treadmill_hr_program_above_range_handler(treadmill);
      } else if (treadmill->hg_lost_tick != 0 && treadmill->hw.handgrip.state == HEARTRATE_HG_LOST && gk_was_connected == 0) {
        treadmill->hg_lost_count++;
        if ((treadmill->training.speed != treadmill->config.speed.min) && (treadmill->hg_lost_count == 5)){
          treadmill_set_speed(treadmill, treadmill->config.speed.min);
          treadmill_set_incline(treadmill, treadmill->config.incline.min);
        }
        else {}
      } 
      else if (gk_was_connected == 1) { //Lucas check for SH needed?
        if (treadmill->hw.ble.gk.connected == 0 && treadmill->hw.ble.sh.connected == 0){
          gk_lost_bpm++;
          if (gk_lost_bpm == 4 && (treadmill->training.incline != 0)){
            treadmill_set_incline(treadmill, treadmill->config.incline.min);
          }
          else if (gk_lost_bpm == 5){
            treadmill_set_speed(treadmill, treadmill->config.speed.min);
          }	
        }			
      }
      else if ((ble_strap_was_connected == 1) && (treadmill->hw.ble.hrm.status == HRM_STATUS_DISCONNECTED)) {
        ble_strap_lost_bpm++;
        if (ble_strap_lost_bpm == 4 && (treadmill->training.incline != 0)){
          treadmill_set_incline(treadmill, treadmill->config.incline.min);
        }
        else if (ble_strap_lost_bpm == 5 && (treadmill->training.speed != treadmill->config.speed.min)){
          treadmill_set_speed(treadmill, treadmill->config.speed.min);
        }				
      }
      else {
        treadmill_clear_dm_message(treadmill);
        treadmill_clear_bpm_hg_loss(treadmill);
      }
      p_fat_burn->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      p_fat_burn->tick_ms = treadmill->get_time_ms();
      break;
    }
  }

  if (treadmill->training.seconds >= p_fat_burn->seconds) {
    treadmill_finish_program(treadmill);
  }
}

static void update_program_super_cardio(treadmill_t *treadmill) {
  struct p_super_cardio *p_super_cardio = &treadmill->program.p_super_cardio;

  treadmill->training.total_seconds = p_super_cardio->seconds;
  treadmill->training.remaining_seconds = treadmill->training.total_seconds - treadmill->training.seconds;

  switch (p_super_cardio->hr_program_state) {
    case HEART_RATE_PROGRAM_STATE_INIT: {
      p_super_cardio->tick_ms = treadmill->get_time_ms();
      p_super_cardio->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      break;
    }
    case HEART_RATE_PROGRAM_STATE_WAIT: {
      if (treadmill->get_time_ms() - p_super_cardio->tick_ms >= 10000) {
        p_super_cardio->hr_program_state = HEART_RATE_PROGRAM_STATE_ACTION;
        p_super_cardio->tick_ms = 0;
      }
      break;
    }
    case HEART_RATE_PROGRAM_STATE_ACTION: {
      if (treadmill->training.bpm && treadmill->training.bpm < p_super_cardio->low_bpm) {
        treadmill_hr_program_below_range_handler(treadmill);
      } else if (treadmill->training.bpm && treadmill->training.bpm > p_super_cardio->high_bpm) {
        treadmill_hr_program_above_range_handler(treadmill);
      } else if (treadmill->hg_lost_tick != 0 && treadmill->hw.handgrip.state == HEARTRATE_HG_LOST && gk_was_connected == 0) {
        treadmill->hg_lost_count++;
        if ((treadmill->training.speed != treadmill->config.speed.min) && (treadmill->hg_lost_count == 5)){
          treadmill_set_speed(treadmill, treadmill->config.speed.min);
          treadmill_set_incline(treadmill, treadmill->config.incline.min);
        }
        else {}
      } 
      else if (gk_was_connected == 1) { //Lucas check for SH needed?
        if (treadmill->hw.ble.gk.connected == 0 && treadmill->hw.ble.sh.connected == 0){
          gk_lost_bpm++;
          if (gk_lost_bpm == 4 && (treadmill->training.incline != 0)){
            treadmill_set_incline(treadmill, treadmill->config.incline.min);
          }
          else if (gk_lost_bpm == 5){
            treadmill_set_speed(treadmill, treadmill->config.speed.min);
          }	
        }			
      }
      else if ((ble_strap_was_connected == 1) && (treadmill->hw.ble.hrm.status == HRM_STATUS_DISCONNECTED)) {
        ble_strap_lost_bpm++;
        if (ble_strap_lost_bpm == 4 && (treadmill->training.incline != 0)){
          treadmill_set_incline(treadmill, treadmill->config.incline.min);
        }
        else if (ble_strap_lost_bpm == 5 && (treadmill->training.speed != treadmill->config.speed.min)){
          treadmill_set_speed(treadmill, treadmill->config.speed.min);
        }				
      }
      else {
        treadmill_clear_dm_message(treadmill);
        treadmill_clear_bpm_hg_loss(treadmill);
      }
      p_super_cardio->hr_program_state = HEART_RATE_PROGRAM_STATE_WAIT;
      p_super_cardio->tick_ms = treadmill->get_time_ms();
      break;
    }
  }

  if (treadmill->training.seconds >= p_super_cardio->seconds) {
    treadmill_finish_program(treadmill);
  }
}

program_training_callback_t program_training_callbacks[] = {
    {.id = PROGRAM_NONE, .update = update_program_none}, //////// time limit
    {.id = PROGRAM_TIME, .update = update_program_time},
    {.id = PROGRAM_DISTANCE, .update = update_program_distance},
    {.id = PROGRAM_CALORIES, .update = update_program_calories},
    {.id = PROGRAM_HIIT, .update = update_program_hiit},
    {.id = PROGRAM_WALK_30MIN, .update = update_program_walk_30min},
    {.id = PROGRAM_FAT_BURN, .update = update_program_fat_burn},
    {.id = PROGRAM_SUPER_CARDIO, .update = update_program_super_cardio},
    {.id = PROGRAM_WALK_08MIN, .update = update_program_walk_08min},
    {.id = PROGRAM_200K_CALORIES, .update = update_program_200k_calories},
};

void treadmill_scene_training_programs_update(treadmill_t *treadmill) {
  for (program_id_t i = 0; i < PROGRAM_TOTALS; ++i) {
    if (program_training_callbacks[i].id == treadmill->program.id) {
      if (program_training_callbacks[i].update != NULL) program_training_callbacks[i].update(treadmill);
      return;
    }
  }
}
