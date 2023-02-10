#include "treadmill_action.h"

#include <string.h>

#include "nvm/nvm.h"
#include "treadmill_programs.h"

void treadmill_start_training_countdown(treadmill_t *treadmill) {
  treadmill->countdown.value = START_TRAINING_COUNTDOWN_VALUE;
  treadmill->countdown.ms = 0;
  treadmill_set_scene(treadmill, SCENE_COUNTDOWN);
}

void treadmill_start_training(treadmill_t *treadmill) {
  memset(&treadmill->training, 0x00, sizeof(struct training));
  treadmill->training.ms = treadmill->ms;
  treadmill_set_speed(treadmill, treadmill->config.speed.min);
  treadmill_set_scene(treadmill, SCENE_TRAINING);
}

void treadmill_abort_countdown(treadmill_t *treadmill) {
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

void treadmill_pause_training(treadmill_t *treadmill) {
  treadmill_set_speed(treadmill, 0);
  treadmill_set_scene(treadmill, SCENE_PAUSED);
}

void treadmill_resume_training(treadmill_t *treadmill) {
  treadmill->training.speed = treadmill->config.speed.min;
  treadmill->training.ms = treadmill->ms;
  treadmill_set_scene(treadmill, SCENE_TRAINING);
}

void treadmill_finish_training(treadmill_t *treadmill) {
  treadmill_set_speed(treadmill, 0);
  treadmill_set_scene(treadmill, SCENE_REPORT);
}

void treadmill_finish_program(treadmill_t *treadmill) {
  treadmill_finish_training(treadmill);
}

static void treadmill_save_training(treadmill_t *treadmill) {
  treadmill->total.distance_in_meters += treadmill->training.meters;
  treadmill->total.time_in_seconds += treadmill->training.seconds;
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS,
                              treadmill->total.distance_in_meters);
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_TIME_SECONDS,
                              treadmill->total.time_in_seconds);
}

void treadmill_close_report(treadmill_t *treadmill) {
  treadmill_save_training(treadmill);
  memset(&treadmill->training, 0x00, sizeof(struct training));
  treadmill->program.id = PROGRAM_NONE;
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

void treadmill_speed_increment(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed + treadmill->config.speed.step <=
          treadmill->config.speed.max) {
    treadmill_set_speed(
        treadmill, treadmill->training.speed + treadmill->config.speed.step);
  }
}

void treadmill_speed_decrement(treadmill_t *treadmill) {
  if (treadmill->training.speed >= treadmill->config.speed.min &&
      treadmill->training.speed - treadmill->config.speed.step >=
          treadmill->config.speed.min) {
    treadmill_set_speed(
        treadmill, treadmill->training.speed - treadmill->config.speed.step);
  }
}

int treadmill_set_speed(treadmill_t *treadmill, const uint16_t speed) {
  if (speed > treadmill->config.speed.max) return 0;
  if (speed > 0 && speed < treadmill->config.speed.min) return 0;
  treadmill->training.speed = speed;
  treadmill->training.pace = treadmill_calculate_pace(treadmill);
  return 1;
}

int treadmill_set_incline(treadmill_t *treadmill, const int16_t incline) {
  if (incline > treadmill->config.incline.max) return 0;
  if (incline < treadmill->config.incline.min) return 0;
  treadmill->training.incline = incline;
  return 1;
}

void treadmill_incline_increment(treadmill_t *treadmill) {
  if (treadmill->training.incline >= treadmill->config.incline.min &&
      treadmill->training.incline + treadmill->config.incline.step <=
          treadmill->config.incline.max) {
    treadmill->training.incline += treadmill->config.incline.step;
  }
}

void treadmill_incline_decrement(treadmill_t *treadmill) {
  if (treadmill->training.incline >= treadmill->config.incline.min &&
      treadmill->training.incline - treadmill->config.incline.step >=
          treadmill->config.incline.min) {
    treadmill->training.incline -= treadmill->config.incline.step;
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
  else
    treadmill->program.id++;
  treadmill->program.param_index = 0;
  program_load_default_value(treadmill);
}

void treadmill_exit_program(treadmill_t *treadmill) {
  memset(&treadmill->program, 0x00, sizeof(struct program));
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

void treadmill_start_program(treadmill_t *treadmill) {
  treadmill_start_training_countdown(treadmill);
}

void treadmill_config(treadmill_t *treadmill) {
  memcpy(&treadmill->previous_config, &treadmill->config,
         sizeof(struct config));
  treadmill_set_scene(treadmill, SCENE_CONFIG);
}

void treadmill_exit_config(treadmill_t *treadmill) {
  treadmill_save_config(treadmill);
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

static int valid_ns(const char *nsu, const int length) { return 1; }

int treadmill_save_nsu(treadmill_t *treadmill, const char *nsu,
                       const int length) {
  if (!valid_ns(nsu, length)) return 0;
  strcpy(treadmill->config.nsu, nsu);
  nvm.NVM_WriteString(NVM_NSU_START_ADDRESS, nsu, length, NVM_NS_MAX_LENGTH);
  return 1;
}

int treadmill_save_nsp(treadmill_t *treadmill, const char *nsp,
                       const int length) {
  if (!valid_ns(nsp, length)) return 0;
  strcpy(treadmill->config.nsp, nsp);
  nvm.NVM_WriteString(NVM_NSP_START_ADDRESS, nsp, length, NVM_NS_MAX_LENGTH);
  return 1;
}

int treadmill_save_nsi(treadmill_t *treadmill, const char *nsi,
                       const int length) {
  if (!valid_ns(nsi, length)) return 0;
  strcpy(treadmill->config.nsp, nsi);
  nvm.NVM_WriteString(NVM_NSI_START_ADDRESS, nsi, length, NVM_NS_MAX_LENGTH);
  return 1;
}

int treadmill_save_total_distance(treadmill_t *treadmill,
                                  uint32_t total_distance_in_meters) {
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS,
                              total_distance_in_meters);
  return 1;
}

int treadmill_save_total_time(treadmill_t *treadmill,
                              uint32_t total_time_in_seconds) {
  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_TIME_SECONDS,
                              total_time_in_seconds);
  return 1;
}

int treadmill_start_or_resume(treadmill_t *treadmill) {
  // todo: implementation

  return 1;
}

int treadmill_stop_procedure(treadmill_t *treadmill) {
  // todo: implementation

  return 1;
}

int treadmill_pause_procedure(treadmill_t *treadmill) {
  // todo: implementation

  return 1;
}
