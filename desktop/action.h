#ifndef ACTION_H
#define ACTION_H
#include "treadmill.h"

void treadmill_program(treadmill_t *treadmill);
void treadmill_start_training_countdown(treadmill_t *treadmill);
void treadmill_start_training(treadmill_t *treadmill);
void treadmill_abort_countdown(treadmill_t *treadmill);
void treadmill_pause_training(treadmill_t *treadmill);
void treadmill_resume_training(treadmill_t *treadmill);
void treadmill_finish_training(treadmill_t *treadmill);
void treadmill_close_report(treadmill_t *treadmill);

void treadmill_speed_increment(treadmill_t *treadmill);
void treadmill_speed_decrement(treadmill_t *treadmill);
void treadmill_incline_increment(treadmill_t *treadmill);
void treadmill_incline_decrement(treadmill_t *treadmill);

void treadmill_next_program(treadmill_t *treadmill);
void treadmill_exit_program(treadmill_t *treadmill);
void treadmill_start_program(treadmill_t *treadmill);

#endif
