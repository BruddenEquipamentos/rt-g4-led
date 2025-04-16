#ifndef TREADMILL_H
#define TREADMILL_H

#include <stdint.h>

#include "beep.h"
#include "ble/ble.h"
#include "treadmill_key.h"
#include "treadmill_programs.h"

#define TREADMILL_INCL_TINE_SHOWN 2000 // TIME WHICH INCLINATION APPEARS ON DOT MATRIX 
#define TREADMILL_INCL_TIME 30000 // TIME BETWEEN INCLINATION APPEARS ON THE DOT MATRIX WHEN DIFF THAN ZERO

#define NVM_U8_MODEL 0x0001
#define NVM_U32_TOTAL_DISTANCE_METERS 0x0002
#define NVM_U32_TOTAL_TIME_SECONDS 0x0003
#define NVM_U8_WEIGHT 0x0004
#define NVM_U32_LUB_DISTANCE 0x0005
#define NVM_U8_AGE 0x0006
#define NVM_U32_LUB_DISTANCE_PARAM 0x0007
#define NVM_U32_MAX_TIME_PARAM 0x0008 /////////////////// SAVE MAX TIME CONFIG
#define NVM_U8_LANG 0x0009 //Lucas 14/04 language
#define NVM_U8_UNIT 0x0010 //Lucas 14/04 unit

// Exemplo de NSU "818790500000007067703"
#define NVM_NS_MAX_LENGTH (22)
#define NVM_NSU_START_ADDRESS (100)
#define NVM_NSP_START_ADDRESS (NVM_NSU_START_ADDRESS + NVM_NS_MAX_LENGTH)
#define NVM_NSI_START_ADDRESS (NVM_NSP_START_ADDRESS + NVM_NS_MAX_LENGTH)
#define NVM_NS_END_ADDRESS (NVM_NSI_START_ADDRESS + NVM_NS_MAX_LENGTH)

#define START_TRAINING_COUNTDOWN_VALUE (3)
#define LUB_NOTIFICATION_SCENE_TIME (5000)
#define IDLE_SCENE_TIME (10000)
#define DEFAULT_WEIGHT_IN_KG (70)
#define DEFAULT_LUB_DISTANCE_PARAM (0)
#define DEFAULT_AGE (30)
#define LUB_NOTIFICATION_DISTANCE_REMAINING_KM (100)

typedef enum inverter_comm_type {
  MOTOR_COMMUNICATION_TYPE_FREQ = 0,
  MOTOR_COMMUNICATION_TYPE_UART = 1,
} inverter_comm_type_t;

typedef enum uart_protocol_state {
  UART_PROTOCOL_INIT_STATE = 0,
  UART_PROTOCOL_DETECTION_STATE,
  UART_PROTOCOL_PROCESS_MODBUS_STATE,
  UART_PROTOCOL_PROCESS_MOVEMENT_STATE,
  UART_PROTOCOL_PROCESS_UNKNOWN,
} uart_protocol_state_t;

typedef enum treadmill_scene {
  SCENE_RESET = 0,
  SCENE_INIT,
  SCENE_IDLE,
  SCENE_EMERGENCY,
  SCENE_LUB,
  SCENE_CONFIG,
  SCENE_PROGRAM,
  SCENE_COUNTDOWN,
  SCENE_TRAINING,
  SCENE_PAUSED,
  SCENE_REPORT,
  SCENE_TOTALS,
} treadmill_scene_t;

enum handgrip_state {
  HEARTRATE_HG_DISABLED = 0,
  HEARTRATE_HG_INITING,
  HEARTRATE_HG_READING,
  HEARTRATE_HG_LOST, // Fat Burn and Super Cardio, ISO 957-6
};

extern char *scene_name[];

typedef struct treadmill {
  treadmill_scene_t scene;
  treadmill_scene_t previous_scene;

  uint32_t scene_tick;  // tick since scene started
  uint32_t idle_tick;   // tick since last keypress
  uint32_t hg_lost_tick; // tick since lost handgrip signal
  uint8_t hg_lost_count; // count to set speed to config.min (aprox. 50 seconds)

  struct program {
    program_id_t id;
    struct p_time p_time;
    struct p_distance p_distance;
    struct p_calories p_calories;
    struct p_hiit p_hiit;
    struct p_walk_30min p_walk_30min;
    struct p_fat_burn p_fat_burn;
    struct p_super_cardio p_super_cardio;
    struct p_walk_08min p_walk_08min;
    struct p_200k_calories p_200k_calories;
    uint8_t param_index;
  } program;

  struct training {
    uint32_t ms;
    uint32_t seconds;

    uint32_t cm;
    uint32_t meters;
    uint32_t kcal;
    uint32_t kcal_gk;

    uint16_t speed;
    uint16_t speed_max; /////////////
    uint16_t average_speed; //////
    int16_t incline;
    // New average speed -- Lucas
		uint32_t speed_sum;
    uint32_t speed_samples;

    uint16_t pace;
    uint16_t pace_max; ///////////////
    uint8_t bpm;

    uint32_t pace_total_seconds;

    // TODO: refactor
    uint32_t remaining_seconds;
    uint32_t total_seconds;
    uint32_t conclusion_percentage;

    uint32_t bpm_total;
    uint32_t bpm_count;
    uint32_t bpm_average;
    uint32_t bpm_max;
  } training;

  // TODO: refactor this struct
  struct aux {
    uint16_t speed_target;
    uint16_t speed_reached;
    uint32_t speed_ms;

    uint32_t incline_changed_time_ms;
    uint32_t last_incline_show;

    // TEMP
    struct average_history {
      uint32_t minute;
      uint16_t average_speed;
      uint16_t height;
    } average_history[16];

    uint16_t average_history_index;

  } aux;

  struct config {
    struct {
      uint16_t min;
      uint16_t max;
      uint16_t step;
    } speed;
    struct {
      int16_t min;
      int16_t max;
      int16_t step;
    } incline;

    int option;
    int model;
    int weight;
    int age;
    uint32_t lub_param_km;
    uint32_t lub_distance_m;
    int modbus;
    int test;
    int max_time; /////// maximum time on quick mode

    char nsu[22];
    char nsp[22];  // legado
    char nsi[22];  // legado

    char device_name[15];

    int language;
    int unit;

  } config;

  struct total {
    uint32_t distance_in_meters;
    uint32_t time_in_seconds;
  } total;

  struct config previous_config;
  struct {
    uint8_t value;
    uint16_t speed;
    uint32_t meters;
    int16_t incline;
    uint32_t ms;
    uint32_t max_time;
  } countdown;

  struct debug {
    uint8_t print_states;
  } debug;

  struct hw {
    void (*beep)(beep_t beep);
    void (*update_rtc)(const uint32_t ms);
    void (*wait_ms)(const int ms);

    ble_t ble;

    struct inverter {
      int (*send_data)(const uint8_t *data, int length);
      int (*recv_data)(uint8_t *data, int max_length);
      int (*change_frequency)(uint16_t period);
      inverter_comm_type_t comm_type;
      uart_protocol_state_t protocol;
    } inverter;

    struct {
      enum handgrip_state state;
    } handgrip;

  } hw;

  struct {
    char data[32];
    uint32_t tick;
    uint32_t duration;
    uint32_t blink_interval;
  } message;

  uint32_t (*get_time_ms)(void);
  uint32_t ms;

  kbd_key_t key_pressed;
  uint32_t key_pressed_tick;
  uint32_t key_pressed_last_tick;

  // int (*on_speed_changed_callback)(treadmill_t *t, const uint16_t speed);
  // int (*on_incline_changed_callback)(treadmill_t *t, const int16_t incline);
  // int (*on_state_changed_callback)(treadmill_t *t, const int state); // state/scene

  int (*on_update_fms_callback)(treadmill_t *t, const uint8_t, const void *param);

} treadmill_t;

void treadmill_setup(treadmill_t *treadmill);

void treadmill_key_press(treadmill_t *treadmill, const kbd_key_t key);
void treadmill_update(treadmill_t *treadmill);

void treadmill_update(treadmill_t *treadmill);
void treadmill_set_scene(treadmill_t *treadmill, treadmill_scene_t scene);

uint32_t treadmill_calculate_pace(const uint16_t speed);
uint32_t treadmill_calculate_calories(treadmill_t *treadmill);

void treadmill_load_config(treadmill_t *treadmill);
void treadmill_save_config(treadmill_t *treadmill);

void treadmill_emergency_pressed(treadmill_t *treadmill);

void treadmill_set_dm_message(treadmill_t *treadmill, const char *text, const uint32_t duration_ms,
                              const uint32_t blink_interval);

void treadmill_clear_dm_message(treadmill_t *treadmill);

uint32_t treadmill_calculate_lub_distance_remaining(treadmill_t *treadmill);

void treadmill_reset_lub(treadmill_t *treadmill);

void treadmill_key_press_clear(treadmill_t *treadmill);

void treadmill_ble_restart(treadmill_t *treadmill);

uint32_t treadmill_current_average_speed(treadmill_t* treadmill);

const char *get_string(treadmill_t* treadmill, int id); /////////

#endif
