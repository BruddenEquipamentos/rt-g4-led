#ifndef TREADMILL_ACTION_H
#define TREADMILL_ACTION_H
#include "treadmill.h"

void treadmill_program(treadmill_t *treadmill);
void treadmill_start_training_countdown(treadmill_t *treadmill);
void treadmill_start_training(treadmill_t *treadmill);
void treadmill_abort_countdown(treadmill_t *treadmill);
void treadmill_pause_training(treadmill_t *treadmill);
void treadmill_reset_training(treadmill_t *treadmill);
void treadmill_resume_training(treadmill_t *treadmill);
void treadmill_finish_training(treadmill_t *treadmill);
void treadmill_finish_program(treadmill_t *treadmill);
void treadmill_close_report(treadmill_t *treadmill);

void treadmill_speed_increment(treadmill_t *treadmill);
void treadmill_speed_decrement(treadmill_t *treadmill);
void treadmill_incline_increment(treadmill_t *treadmill);
void treadmill_incline_decrement(treadmill_t *treadmill);

int treadmill_set_speed(treadmill_t *treadmill, const uint16_t speed);
int treadmill_set_incline(treadmill_t *treadmill, const int16_t incline);

void treadmill_next_program(treadmill_t *treadmill);
void treadmill_exit_program(treadmill_t *treadmill);
void treadmill_start_program(treadmill_t *treadmill);

void treadmill_config(treadmill_t *treadmill);
void treadmill_exit_config(treadmill_t *treadmill);

int treadmill_save_nsu(treadmill_t *treadmill, const char *nsu,
                       const int length);
int treadmill_save_nsp(treadmill_t *treadmill, const char *nsp,
                       const int length);
int treadmill_save_nsi(treadmill_t *treadmill, const char *nsi,
                       const int length);
int treadmill_save_total_distance(treadmill_t *treadmill,
                                  uint32_t total_distance_in_meters);
int treadmill_save_total_time(treadmill_t *treadmill,
                              uint32_t total_time_in_seconds);

int treadmill_start_or_resume(treadmill_t *treadmill);
int treadmill_pause_procedure(treadmill_t *treadmill);
int treadmill_stop_procedure(treadmill_t *treadmill);

int treadmill_send_ftms_data(treadmill_t *treadmill);

void treadmill_set_bpm_read(treadmill_t *treadmill);
void treadmill_set_bpm_value(treadmill_t *treadmill, uint8_t bpm);
void treadmill_set_bpm_stop(treadmill_t *treadmill, uint8_t in_program);

void treadmill_clear_bpm_hg_loss(treadmill_t *treadmill);

#endif
