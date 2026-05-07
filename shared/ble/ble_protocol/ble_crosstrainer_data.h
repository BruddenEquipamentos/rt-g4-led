#ifndef BLE_CROSSTRAINER_DATA_H
#define BLE_CROSSTRAINER_DATA_H

#define FTMS_CROSSTRAINER_DATA_UUID 0x2ACE

typedef enum ble_crosstrainer_data_fields {
    CTD_MORE_DATA_PRESENT                   = 1<<0,
    CTD_AVERAGE_SPEED_PRESENT               = 1<<1,
    CTD_TOTAL_DISTANCE__PRESENT             = 1<<2,
    CTD_STEP_COUNT_PRESENT                  = 1<<3,
    CTD_STRIDE_COUNT_PRESENT                = 1<<4,
    CTD_ELEVATION_GAIN_PRESENT              = 1<<5,
    CTD_INCLINATION_AND_RAMP_ANGLE__PRESENT = 1<<6,
    CTD_RESISTANCE_LEVEL_PRESENT            = 1<<7,
    CTD_INSTANTANEOUS_POWER_PRESENT         = 1<<8,
    CTD_AVERAGE_POWER_PRESENT               = 1<<9,
    CTD_EXPENDED_ENERGY_PRESENT             = 1<<10,
    CTD_HEART_RATE_PRESENT                  = 1<<11,
    CTD_METABOLIC_EQUIVALENT_PRESENT        = 1<<12,
    CTD_ELAPSED_TIME_PRESENT                = 1<<13,
    CTD_REMAINING_TIME_PRESENT              = 1<<14,
    CTD_MOVEMENT_DIRECTION                  = 1<<15,
} ble_crosstrainer_data_fields_t;

#endif
