#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define DISPLAY_FIELD_BUFFER_MAX_LENGTH 8

typedef struct display_fields {
    char output[DISPLAY_FIELD_BUFFER_MAX_LENGTH];
    struct {
        uint32_t interval;
        uint32_t time_ms;
    } blink;
} display_fields_t;

typedef struct display {
    display_fields_t distance;
    display_fields_t time;
    display_fields_t speed;
    display_fields_t kcal;
    display_fields_t program;
    display_fields_t pace;
    display_fields_t incline;
    display_fields_t bpm;
} display_t;

#endif
