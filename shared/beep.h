/*
 * beep.h
 *
 *  Created on: Jan 18, 2023
 *      Author: fminoru
 */

#ifndef SRC_BEEP_H_
#define SRC_BEEP_H_

typedef enum beep {
    BEEP_KEYPRESS_ACK=0,
    BEEP_KEYPRESS_NAK,
    BEEP_COUNTDOWN_COUNT,
    BEEP_COUNTDOWN_END,
    BEEP_REST,
    BEEP_PROGRAM_FINISHED,
    BEEP_EMERGENCY,
} beep_t;

#endif /* SRC_BEEP_H_ */
