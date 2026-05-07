#ifndef BLE_TREADMILL_DATA_H
#define BLE_TREADMILL_DATA_H

#define FTMS_TREADMILL_DATA_UUID 0x2ACD

typedef enum treadmill_data {
  TD_MORE_DATA = 1 << 0,                                // resolution 0.01 km/h
  TD_AVERAGE_SPEED_PRESENT = 1 << 1,                    // resolution 0.01 km/h
  TD_TOTAL_DISTANCE_PRESENT = 1 << 2,                   // resolution 1 meter
  TD_INCLINATION_RAMP_ANGLE_SETTING_PRESENT = 1 << 3,   // resolution 0.1 %
  TD_ELEVATION_GAIN_PRESENT = 1 << 4,                   // resolution 0.1 m
  TD_INSTANTANEOUS_PACE_PRESENT = 1 << 5,               // resolution 0.1 km/m
  TD_AVERAGE_PACE_PRESENT = 1 << 6,                     // resolution 0.1 km/m
  TD_EXPENDED_ENERGY_PRESENT = 1 << 7,                  // resolution 1 kcal
  TD_HEART_RATE_PRESENT = 1 << 8,                       // resolution 1 bpm
  TD_METABOLIC_EQUIVALENT_PRESENT = 1 << 9,             // resolution 0.1 mer
  TD_ELAPSED_TIME_PRESENT = 1 << 10,                    // resolution of 1 sec
  TD_REMAINING_TIME_PRESENT = 1 << 11,                  // resolution of 1 sec
  TD_FORCE_ON_BELT_AND_POWER_OUTPUT_PRESENT = 1 << 12,  // resolution of 1
} treadmill_data_fields_t;

// new fields
enum FMStatus {
    ReservedForFutureUse = 0x00,
    Reset = 0x01,
    StoppedOrPausedByTheUser = 0x02,
    StoppedBySafetyKey = 0x03,
    StartedOrResumedByTheUser = 0x04,
    TargetSpeedChanged = 0x05,
    TargetInclineChanged = 0x06,
    TargetResistanceLevelChanged = 0x07,
    TargedPowerChanged = 0x08,
    TargetHearRateChanged = 0x09,
    TargetExpendedEnergyChanged = 0x0A,
    TargetedNumberOfStepsChanged = 0x0B,
    TargetedNumberOfStridesChanged = 0x0C,
    TargetedDistanceChanged = 0x0D,
    TargetedTrainingTimeChanged = 0x0E,
    TargetedTimeInTwoHeartRateZonesChanged = 0x0F,
    TargetedTimeInThreeHeartRateZonesChanged = 0x10,
    TargetedTimeInFiveHeartRateZonesChanged = 0x11,
    IndoorBikeSimulationParametersChanged = 0x12,
    WheelCircumferenceChanged = 0x13,
    SpinDownStatus = 0x14,
    TargetedCadenceChanged = 0x15,
};



#endif
