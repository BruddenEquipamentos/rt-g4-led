#ifndef ACTION_H
#define ACTION_H
#include "treadmill.h"

void treadmill_start_training(treadmill_t *treadmill);
void treadmill_abort_countdown(treadmill_t *treadmill);
void treadmill_pause_training(treadmill_t *treadmill);
void treadmill_resume_training(treadmill_t *treadmill);
void treadmill_finish_training(treadmill_t *treadmill);
void treadmill_close_report(treadmill_t *treadmill);

#endif