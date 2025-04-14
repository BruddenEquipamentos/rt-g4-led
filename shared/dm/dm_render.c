#include "dm_render.h"

#include <string.h>

#include "../panel/rt_led.h"
#include "../treadmill_motor_modbus.h"
#include "dm.h"

#include "../sd/sd_render.h"

#include "treadmill_programs.h"
#include "treadmill_programs_update.h" /////////////////

#include "lang.h" //Language options

// aux variables
unsigned long end_tick = 0;
int render_finished = 0;
int report_goal = 0; 

// extern variables
extern uint16_t on_interval; 
extern int report_state; 
extern int completed_goal; 
extern interval_t hiit_intervals[20];
extern uint8_t aux_teste;
extern int incline_limit_reached;

static void dm_render_all(void){
  for (int col = 0; col < 15; ++col) {
    for (int row = 0; row < 5; ++row) {
      dm_buffer[row] |= (1 << (15 - col));
    }
  }
}

static int dm_blink(const uint32_t update_time_ms, const uint32_t blink_interval_ms) {
  static uint32_t dm_blink_time_ms = 0;
  if (blink_interval_ms == 0) return 0;
  if (update_time_ms - dm_blink_time_ms > blink_interval_ms) {
    dm_blink_time_ms = update_time_ms;
    return 1;
  } else if (update_time_ms - dm_blink_time_ms > blink_interval_ms / 2) {
    return 0;
  }
  return 1;
}

static void dm_render_mb_param_value(treadmill_t *treadmill) {
  mb_operation_t *op = modbus_get_current_selected_param();
  dm_render_text(op->rfmtd, 16);
}

static int dm_get_height_for_speed(const uint16_t speed) {
  static const uint16_t speeds[5] = {500, 800, 1100, 1400, 1800};
  static const int heights[5] = {0, 1, 2, 3, 4};
  if (speed < speeds[0])
    return heights[0];
  else if (speed < speeds[1])
    return heights[1];
  else if (speed < speeds[2])
    return heights[2];
  else if (speed < speeds[3])
    return heights[3];
  else
    return heights[4];
}

//// frametime capture
static int dm_get_frame_from_speed(const uint16_t speed) {
  static const uint16_t speeds[6] = {300, 600, 900, 1200, 1500, 1800};
  // static const int frames[6] = {600, 480, 430, 300, 270, 100};
  static const int frames[6] = {500, 400, 350, 300, 250, 100};
  if (speed < speeds[0])
    return frames[0];
  else if (speed < speeds[1])
    return frames[1];
  else if (speed < speeds[2])
    return frames[2];
  else if (speed < speeds[3])
    return frames[3];  
  else if (speed < speeds[4])
    return frames[4];
  else
    return frames[5];
}

// static void dm_render_time(treadmill_t *treadmill) {
//   int height = dm_get_height_for_speed(treadmill->training.speed);

//   for (int i = 0; i < 16; i++) {
//     if (i == treadmill->aux.average_history_index) {
//       dm_render_column_with_height(i, height, treadmill->get_time_ms(), 800);
//     } else {
//       int h = dm_get_height_for_speed(treadmill->aux.average_history[i].average_speed);
//       dm_render_column_with_height(i, h, 0, 0);
//     }
//   }
// }

static void dm_render_default(treadmill_t *treadmill) {
  int height = dm_get_height_for_speed(treadmill->training.speed);

  for (int i = 0; i < 16; i++) {
    if (i == treadmill->aux.average_history_index) {
      dm_render_column_with_height(i, height, treadmill->get_time_ms(), 800);
    } 
    else if (i < treadmill->aux.average_history_index) {
      int h = dm_get_height_for_speed(treadmill->aux.average_history[i].average_speed);
      dm_render_column_with_height(i, h, 0, 0);
    }
    else{ //do nothing on future dots
    }
  }
}

uint16_t hiit_speeds[20] = {0};

void dm_render_hiit(treadmill_t *treadmill, uint16_t interval, interval_t *hiit_intervals) {

  int height = dm_get_height_for_speed(treadmill->training.speed);
  hiit_speeds[interval] = height;

  if (interval <= 15){
    if (treadmill->training.seconds <= hiit_intervals[interval].seconds) {
      dm_render_column_with_height(interval, height, treadmill->get_time_ms(), 800);
    }
    for (int i = 0; i < interval; i++){
      dm_render_column_with_height(i, hiit_speeds[i], treadmill->get_time_ms(), 0);
    }
  }
  else if (interval > 15) {
    dm_reset_frame();
    if (treadmill->training.seconds <= hiit_intervals[interval].seconds) {
      int dm_interval = interval - 16;
      // dm_render_column_with_height((interval - 16), height, treadmill->get_time_ms(), 800);
      dm_render_column_with_height((dm_interval), height, treadmill->get_time_ms(), 800);
      for (int i = 0; i < 4; i++){
        if (i < dm_interval){
          dm_render_column_with_height(i, hiit_speeds[i + 16], treadmill->get_time_ms(), 0);
        }
        else if (i == dm_interval){
          dm_render_column_with_height(i, hiit_speeds[i + 16], treadmill->get_time_ms(), 800);
        }
        else{}
      }
    }
  } 
}

/// new version for walk program (30 min) ////////////
static void dm_render_time_walk_30(treadmill_t *treadmill) {
  if (treadmill->training.seconds <= (60 * 5))
  {
    dm_render_column_with_height(0, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(1, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(2, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(0, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(1, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(2, 1, treadmill->get_time_ms(), 0);
  } //// interval 0 - 5 min

  if (treadmill->training.seconds > (60 * 5) && treadmill->training.seconds <= (60 * 10))
  {
    dm_render_column_with_height(3, 2, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(4, 2, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(5, 2, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(3, 2, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(4, 2, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(5, 2, treadmill->get_time_ms(), 0);
  } //// interval 5 - 10 min

  if (treadmill->training.seconds > (60 * 10) && treadmill->training.seconds <= (60 * 20))
  {
    dm_render_column_with_height(6, 2, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(7, 2, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(8, 2, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(9, 2, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(6, 4, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(7, 4, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(8, 4, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(9, 4, treadmill->get_time_ms(), 0);
  } //// interval 10 - 20 min

  if (treadmill->training.seconds > (60 * 10) && treadmill->training.seconds <= (60 * 20))
  {
    dm_render_column_with_height(6, 4, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(7, 4, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(8, 4, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(9, 4, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(6, 4, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(7, 4, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(8, 4, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(9, 4, treadmill->get_time_ms(), 0);
  } //// interval 10 - 20 min

  if (treadmill->training.seconds > (60 * 20) && treadmill->training.seconds <= (60 * 25))
  {
    dm_render_column_with_height(10, 2, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(11, 2, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(12, 2, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(10, 2, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(11, 2, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(12, 2, treadmill->get_time_ms(), 0);
  } //// interval 20 - 25 min

  if (treadmill->training.seconds > (60 * 25) && treadmill->training.seconds <= (60 * 30))
  {
    dm_render_column_with_height(13, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(14, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(15, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(13, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(14, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(15, 1, treadmill->get_time_ms(), 0);
  } //// interval 20 - 25 min
}

/// new version for walk program (08 min) ---- needs rework
static void dm_render_time_walk_08(treadmill_t *treadmill) {
  if (treadmill->training.seconds <= (60 * 1))
  {
    dm_render_column_with_height(0, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(1, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(0, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(1, 1, treadmill->get_time_ms(), 0);
  } //// interval 0 - 1 min

  if (treadmill->training.seconds > (60 * 1) && treadmill->training.seconds <= (60 * 2))
  {
    dm_render_column_with_height(2, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(3, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(2, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(3, 1, treadmill->get_time_ms(), 0);
  } //// interval 1 - 2 min

  if (treadmill->training.seconds > (60 * 2) && treadmill->training.seconds <= (60 * 3))
  {
    dm_render_column_with_height(4, 3, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(5, 3, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(4, 3, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(5, 3, treadmill->get_time_ms(), 0);
  } //// interval 2 - 3 min

  if (treadmill->training.seconds > (60 * 3) && treadmill->training.seconds <= (60 * 4))
  {
    dm_render_column_with_height(6, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(7, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(6, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(7, 1, treadmill->get_time_ms(), 0);
  } //// interval 3 - 4 min

  if (treadmill->training.seconds > (60 * 4) && treadmill->training.seconds <= (60 * 5))
  {
    dm_render_column_with_height(8, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(9, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(8, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(9, 1, treadmill->get_time_ms(), 0);
  } //// interval 4 - 5 min

  if (treadmill->training.seconds > (60 * 5) && treadmill->training.seconds <= (60 * 6))
  {
    dm_render_column_with_height(10, 3, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(11, 3, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(10, 3, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(11, 3, treadmill->get_time_ms(), 0);
  } //// interval 5 - 6 min

  if (treadmill->training.seconds > (60 * 6) && treadmill->training.seconds <= (60 * 7))
  {
    dm_render_column_with_height(12, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(13, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(12, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(13, 1, treadmill->get_time_ms(), 0);
  } //// interval 6 - 7 min

  if (treadmill->training.seconds > (60 * 7) && treadmill->training.seconds <= (60 * 8))
  {
    dm_render_column_with_height(14, 1, treadmill->get_time_ms(), 800);
    dm_render_column_with_height(15, 1, treadmill->get_time_ms(), 800);
  }
  else
  {
    dm_render_column_with_height(14, 1, treadmill->get_time_ms(), 0);
    dm_render_column_with_height(15, 1, treadmill->get_time_ms(), 0);
  } //// interval 7 - 8 min
}

/// new render for calorie programs (normal and 200kcal)
void dm_render_calories(treadmill_t *treadmill) {
  int calorias = 0;
  switch (treadmill->program.id) {
    case PROGRAM_200K_CALORIES:
      calorias = treadmill->program.p_200k_calories.kcal;
      break;
    case PROGRAM_CALORIES:
      calorias = treadmill->program.p_calories.kcal;
      break;
    default:;
  }
    if ((treadmill->training.kcal / 1000) <= (15 * calorias / 100)){
      dm_render_column_with_height(0, 0, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(1, 0, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(2, 0, treadmill->get_time_ms(), 800);
    }
    else if ((treadmill->training.kcal / 1000) > (15 * calorias / 100)) { // else
      dm_render_column_with_height(0, 0, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(1, 0, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(2, 0, treadmill->get_time_ms(), 0);
    } //// interval 0 - 15%

    if ((treadmill->training.kcal / 1000) >= (16 * calorias / 100) &&
    (treadmill->training.kcal / 1000) <= (30 * calorias / 100)){
      dm_render_column_with_height(3, 1, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(4, 1, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(5, 1, treadmill->get_time_ms(), 800);
    }
    // else{
    else if ((treadmill->training.kcal / 1000) > (30 * calorias / 100)){
      dm_render_column_with_height(3, 1, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(4, 1, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(5, 1, treadmill->get_time_ms(), 0);
    } //// interval 16 - 30%

    if ((treadmill->training.kcal / 1000) >= (31 * calorias / 100) &&
    (treadmill->training.kcal / 1000) <= (50 * calorias / 100)){
      dm_render_column_with_height(6, 2, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(7, 2, treadmill->get_time_ms(), 800);
    }
    // else{
    else if ((treadmill->training.kcal / 1000) > (50 * calorias / 100)){
      dm_render_column_with_height(6, 2, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(7, 2, treadmill->get_time_ms(), 0);
    } //// interval 31 - 50%

    if ((treadmill->training.kcal / 1000) >= (51 * calorias / 100) &&
    (treadmill->training.kcal / 1000) <= (66 * calorias / 100)){
      dm_render_column_with_height(8, 2, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(9, 2, treadmill->get_time_ms(), 800);
    }
    // else{
    else if ((treadmill->training.kcal / 1000) > (66 * calorias / 100)){
      dm_render_column_with_height(8, 2, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(9, 2, treadmill->get_time_ms(), 0);
    } //// interval 51 - 65%

    if ((treadmill->training.kcal / 1000) >= (66 * calorias / 100) &&
    (treadmill->training.kcal / 1000) <= (80 * calorias / 100)){
      dm_render_column_with_height(10, 3, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(11, 3, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(12, 3, treadmill->get_time_ms(), 800);
    }
    // else{
    else if ((treadmill->training.kcal / 1000) > (80 * calorias / 100)){
      dm_render_column_with_height(10, 3, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(11, 3, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(12, 3, treadmill->get_time_ms(), 0);
    } //// interval 66 - 80%

    if ((treadmill->training.kcal / 1000) >= (81 * calorias / 100) &&
    (treadmill->training.kcal / 1000) <= (90 * calorias / 100)){
      dm_render_column_with_height(13, 4, treadmill->get_time_ms(), 800);
      dm_render_column_with_height(14, 4, treadmill->get_time_ms(), 800);
    }
    // else{
    else if ((treadmill->training.kcal / 1000) > (90 * calorias / 100)){
      dm_render_column_with_height(13, 4, treadmill->get_time_ms(), 0);
      dm_render_column_with_height(14, 4, treadmill->get_time_ms(), 0);
    } //// interval 81 - 90%

    if ((treadmill->training.kcal / 1000) > (90 * calorias / 100) &&
    (treadmill->training.kcal / 1000) <= (100 * calorias / 100)){ // (treadmill->training.kcal / 1000) <= (100 * calorias / 100)){
      dm_render_column_with_height(15, 4, treadmill->get_time_ms(), 800);
    }
    // else{
    else if ((treadmill->training.kcal / 1000) > (100 * calorias / 100)){
      dm_render_column_with_height(15, 4, treadmill->get_time_ms(), 0);
    } //// interval 91 - 100%
}

////////////////////////////////////////////////////////////////////////////

static void dm_render_prog_time(treadmill_t *treadmill) {
  uint16_t conclusion_percentage[16] = {0};
  uint8_t dots_by_percentage[16] = {0};

  for (int i = 0; i < 16; i++){
    conclusion_percentage[i] = i*7;
    dots_by_percentage[i] = i;
    if (i == 15){
      conclusion_percentage[i] = 100;
    }
    if (treadmill->training.conclusion_percentage <= 7){
        dm_render_column_with_height(dots_by_percentage[0], 5, treadmill->get_time_ms(), 800);
    }
    else if (treadmill->training.conclusion_percentage >= conclusion_percentage[i]){
      if (treadmill->training.conclusion_percentage > (conclusion_percentage[i] - 7) && treadmill->training.conclusion_percentage < conclusion_percentage[i] + 7){
        dm_render_column_with_height(dots_by_percentage[i], 5, treadmill->get_time_ms(), 800);
      }
      else{
        dm_render_column_with_height(dots_by_percentage[i], 5, treadmill->get_time_ms(), 0);
      }
    }
  }
}

void dm_render_incline(treadmill_t *treadmill){
    char text[16];
    //  sprintf(text, "%1d.%1d%%", treadmill->training.incline / 10, treadmill->training.incline % 10);
    sprintf(text, "%1d%%", treadmill->training.incline / 10);
    int length = dm_compute_length(text);
    if (length < 16) {
      dm_render_text(text, 8 + length / 2);
    }
    else {
      dm_render_text(text, 16);
    }
    return;
}

////////////////////////////////////////////////////////////////////////////

void dm_render(treadmill_t *treadmill) {
  if (treadmill->message.tick) {
    dm_clear_buffer();
    if (!dm_blink(treadmill->get_time_ms(), treadmill->message.blink_interval)) {
      if (dm_compute_length(treadmill->message.data) > 16) {
        dm_render_text_animated(treadmill->message.data, 50, treadmill->get_time_ms());
      } else {
        dm_render_text(treadmill->message.data, 16);
      }
    }
    return;
  }

  switch (treadmill->scene) {
    case SCENE_INIT: {
      dm_render_line(4);
      dm_render_line(3);
      dm_render_line(2);
      dm_render_line(1);
      dm_render_line(0);
      break;
    }
    case SCENE_IDLE: { /////// new message
      if (treadmill->config.modbus) {
        dm_render_mb_param_value(treadmill);
      } else {
        if (treadmill->hw.ble.gk.connected){
          if (treadmill->hw.ble.ftms.subscribed == 1) {
            // Mostra NFC fixo por 1 segundo após subscrição
            if (treadmill->get_time_ms() - treadmill->hw.ble.gk.connected_at <= 4000) {
              dm_render_text("NFC", 14);
            } else {
              dm_render_text_animated("APERTE START OU ESCOLHA UM PROGRAMA", 100, treadmill->get_time_ms());
            }
          } else {
            // Pisca NFC enquanto conectado mas não subscrito
            if (dm_blink(treadmill->get_time_ms(), 1000)) {
              dm_render_text("NFC", 14);
            } else {
              dm_clear_buffer();
            }
          }
        }
        else if (treadmill->hw.ble.sh.connected == 1){
          if (treadmill->hw.ble.ftms.subscribed == 1) {
            // Mostra NFC fixo por 1 segundo após subscrição
            if (treadmill->get_time_ms() - treadmill->hw.ble.sh.connected_at <= 4000) {
              dm_render_text("NFC", 14);
            } else {
              dm_render_text_animated("APERTE START OU ESCOLHA UM PROGRAMA", 100, treadmill->get_time_ms());
            }
          } else {
            // Pisca NFC enquanto conectado mas não subscrito
            if (dm_blink(treadmill->get_time_ms(), 1000)) {
              dm_render_text("NFC", 14);
            } else {
              dm_clear_buffer();
            }
          }
        }
        else if (treadmill->hw.ble.ftms.subscribed && treadmill->hw.ble.hrm.bpm > 0){
            // Pisca NFC enquanto conectado mas não subscrito
            if (dm_blink(treadmill->get_time_ms(), 1000)) {
              dm_render_text("NFC", 14);
            } else {
              dm_clear_buffer();
            }
        }
        else {
          dm_render_text_animated("APERTE START OU ESCOLHA UM PROGRAMA", 100, treadmill->get_time_ms());
        }
        // LED TEST
        // dm_render_all();
      }
      break;
    }
    case SCENE_TRAINING: {
      if (treadmill->config.modbus) {
        dm_render_mb_param_value(treadmill);
      } else {
        dm_clear_buffer();
        
        // Lógica de exibição do NFC
        if (treadmill->hw.ble.gk.connected){
          if (treadmill->hw.ble.ftms.subscribed == 1) {
            // Mostra NFC fixo por 1 segundo após subscrição
            if (treadmill->get_time_ms() - treadmill->hw.ble.gk.connected_at <= 4000) {
              dm_render_text("NFC", 14);
              return;
            }
          } else {
            // Apenas pisca NFC ou limpa a tela, sem mostrar contexto anterior
            if (dm_blink(treadmill->get_time_ms(), 1000)) {
              dm_render_text("NFC", 14);
            } else {
              dm_clear_buffer();
            }
            return; // Retorna aqui para não mostrar o contexto anterior
          }
        }
        else if (treadmill->hw.ble.sh.connected){
          if (treadmill->hw.ble.ftms.subscribed == 1) {
            // Mostra NFC fixo por 1 segundo após subscrição
            if (treadmill->get_time_ms() - treadmill->hw.ble.sh.connected_at <= 4000) {
              dm_render_text("NFC", 14);
              return;
            }
          } else {
            // Apenas pisca NFC ou limpa a tela, sem mostrar contexto anterior
            if (dm_blink(treadmill->get_time_ms(), 1000)) {
              dm_render_text("NFC", 14);
            } else {
              dm_clear_buffer();
            }
            return; // Retorna aqui para não mostrar o contexto anterior
          }
        }

        extern uint8_t __new_display;
        if (__new_display) {
          // if incline has been pressed, show incline for specific time
          if (treadmill->aux.incline_changed_time_ms &&
              treadmill->get_time_ms() - treadmill->aux.incline_changed_time_ms <= TREADMILL_INCL_TINE_SHOWN) {
            treadmill->aux.last_incline_show = treadmill->get_time_ms();  /////////////////////
            dm_render_incline(treadmill);
            return;
          }
          else if ((treadmill->training.incline != 0) && (treadmill->get_time_ms() - treadmill->aux.last_incline_show) >= TREADMILL_INCL_TIME){            
            if (treadmill->get_time_ms() - treadmill->aux.last_incline_show <= TREADMILL_INCL_TIME + TREADMILL_INCL_TINE_SHOWN) {
              dm_render_incline(treadmill);
            }
            else{
              treadmill->aux.last_incline_show = treadmill->get_time_ms();
            }
            return;
          }
          else if (aux_teste == 1){
            dm_render_incline(treadmill);
            // aux_teste = 0;
          }
        }
        switch (treadmill->program.id) {
          case PROGRAM_NONE: { /// render lines according to training speed
            uint32_t velocidade = dm_get_frame_from_speed(treadmill->training.speed);
            dm_render_lines(velocidade, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_TIME: {
            dm_render_prog_time(treadmill);
            break;
          }
          case PROGRAM_200K_CALORIES: /// percentage of calorie target completion
            int meta_50per = treadmill->program.p_200k_calories.kcal / 2;
            if ((treadmill->training.kcal / 1000) >= (meta_50per) && (treadmill->training.kcal / 1000) <= (meta_50per + 2)) {
              dm_render_text_animated("50%% DA META", 70, treadmill->get_time_ms());
            }
            else {
              dm_render_calories(treadmill);
            }
            break;
          case PROGRAM_CALORIES: {
            if ((treadmill->training.kcal / 1000) >= (treadmill->program.p_calories.kcal / 2) && (treadmill->training.kcal / 1000) <= ((treadmill->program.p_calories.kcal / 2) + 2)) {
              dm_render_text_animated("50%% DA META", 70, treadmill->get_time_ms());
            }
            else {
              dm_render_calories(treadmill);
            }
            break;
          }
          case PROGRAM_HIIT: { ///////////////////redo
            dm_render_hiit(treadmill, on_interval, hiit_intervals);
            break;
          }
          case PROGRAM_FAT_BURN:
          case PROGRAM_SUPER_CARDIO: {
            if (treadmill->training.bpm == 0) {
              if (!dm_blink(treadmill->get_time_ms(), 1000)) {
                dm_render_text("@;", 12);
                return;
              }
            }
            else {
              dm_render_time_walk_30(treadmill);
            }
            break;
          }
          case PROGRAM_WALK_30MIN: { /// dm_render_time modified (without blank dots)
            dm_render_time_walk_30(treadmill);
            break;
          }
          case PROGRAM_WALK_08MIN: {
            dm_render_time_walk_08(treadmill);
            break;
          }
          default:
            dm_render_default(treadmill);
            break;
          }
        }
      break;
    }
    case SCENE_EMERGENCY: {
      treadmill_clear_dm_message(treadmill);
      dm_render_text_animated("EMERGENCIA", 70, treadmill->get_time_ms());
      break;
    }
    case SCENE_PAUSED: {
      treadmill_clear_dm_message(treadmill);
      dm_render_text_animated("PAUSA * APERTE STOP PARA ENCERRAR", 70, treadmill->get_time_ms());
      // dm_render_text_animated("PAUSA", 70, treadmill->get_time_ms());
      break;
    }
    // case SCENE_LUB: {
    //   dm_render_text("LUB", 13);
    //   break;
    // }
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
        // case CONFIG_OPTION_LUB: {
        //   dm_render_text("LUB", 16);
        //   break;
//        }  // LUB
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
          if (treadmill->hw.ble.state == BLE_STATE_READY) {
            dm_render_text("BLE", 16);
          } else {
            dm_render_text("S/BL", 16);
          }
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
        case CONFIG_OPTION_MAX_TIME: {
          dm_render_text("TMP", 16);
          break;
        }  // TMP
        default:;
      }
      break;
    }
    case SCENE_PROGRAM: {
        switch(treadmill->program.id){
          case PROGRAM_TIME:{
            dm_render_text_animated("P1 * TEMPO", 100, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_DISTANCE:{
            dm_clear_buffer();
            dm_render_text_animated("P2 * DIST", 100, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_CALORIES:{
            dm_clear_buffer();
            dm_render_text_animated("P3 * CAL", 100, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_HIIT:{
            treadmill->scene_tick = treadmill->get_time_ms();
            if ((treadmill->get_time_ms() - treadmill->scene_tick <= 2 * 1000) && (render_finished == 0)) {
              render_finished = dm_render_text_animated("P4 * INT", 100, treadmill->get_time_ms());
              // treadmill->scene_tick = treadmill->get_time_ms();
            }
            else{
              switch (treadmill->program.param_index) {
                case 0:
                  dm_render_text("T1", 11);
                  break;
                case 1:
                  dm_render_text("V1", 12);
                  break;
                case 2:
                  dm_render_text("T2", 11);
                  break;
                case 3:
                  dm_render_text("V2", 13);
                  break;
                case 4:
                  dm_render_text_animated("SERIES", 100, treadmill->get_time_ms());
                  break;
                default:;
              }
            }
            break;
          }
          case PROGRAM_WALK_30MIN:{
            dm_render_text_animated("P5 * 30 MIN", 100, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_FAT_BURN:{
            treadmill->scene_tick = treadmill->get_time_ms();
            if ((treadmill->get_time_ms() - treadmill->scene_tick <= 2 * 1000) && (render_finished == 0)) {
              render_finished = dm_render_text_animated("P6 * MOD", 100, treadmill->get_time_ms());
              // treadmill->scene_tick = treadmill->get_time_ms();
            }
            else{
              switch (treadmill->program.param_index) {
                case 0:
                  break;
                case 1:
                  dm_render_text_animated("IDADE", 100, treadmill->get_time_ms());
                  break;  
                default:;
              }
            }
            // dm_render_text_animated("P6 * MOD", 100, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_SUPER_CARDIO:{
            treadmill->scene_tick = treadmill->get_time_ms();
            if ((treadmill->get_time_ms() - treadmill->scene_tick <= 2 * 1000) && (render_finished == 0)) {
              render_finished = dm_render_text_animated("P7 * VIG", 100, treadmill->get_time_ms());
              // treadmill->scene_tick = treadmill->get_time_ms();
            }
            else{
              switch (treadmill->program.param_index) {
                case 0:
                  break;
                case 1:
                  dm_render_text_animated("IDADE", 100, treadmill->get_time_ms());
                  break;  
                default:;
              }
            }
            // dm_render_text_animated("P7 * VIG", 100, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_WALK_08MIN:{
            dm_render_text_animated("P8 * 8 MIN", 100, treadmill->get_time_ms());
            break;
          }
          case PROGRAM_200K_CALORIES:{
            dm_render_text_animated("P9 * 200 CAL", 100, treadmill->get_time_ms());
            break;
          }
          default:
            dm_render_text("PROG", 16);
            break;
        }
        break;
    }
    case SCENE_REPORT: {
      treadmill_clear_dm_message(treadmill);
      if (treadmill->program.id == PROGRAM_NONE || (completed_goal == 0)){
        if (report_state == 0) {
          dm_render_text("MAX", 16);
        }       
        else if (report_state == 1) {
          dm_render_text("MED", 14);
        }
      }
      else if (completed_goal == 1) {
        // report_goal = dm_render_text_animated("META CONCLUIDA", 100, treadmill->get_time_ms());
        uint32_t tick_message = treadmill->get_time_ms();
        if ((treadmill->get_time_ms() - tick_message <= 2 * 1000) && (report_goal == 0)) {
          report_goal = dm_render_text_animated("META CONCLUIDA", 100, treadmill->get_time_ms());
        }
        else {
          if (report_state == 0) {
            dm_render_text("MAX", 16);
          }
          else if (report_state == 1) {
            dm_render_text("MED", 16);
          }
        }
      }
      break;
    }
    default: {
      if (treadmill->scene == SCENE_COUNTDOWN && treadmill->countdown.value != 0) {
        dm_render_number(treadmill->countdown.value);
      } else {
        dm_clear_buffer();
      }
    };
  }
}
