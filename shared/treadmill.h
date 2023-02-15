#ifndef TREADMILL_H
#define TREADMILL_H

#include <stdint.h>

#include "beep.h"
#include "ble/ble.h"
#include "treadmill_key.h"
#include "treadmill_programs.h"

#define NVM_U8_MODEL 0x0001
#define NVM_U32_TOTAL_DISTANCE_METERS 0x0002
#define NVM_U32_TOTAL_TIME_SECONDS 0x0003
#define NVM_U8_WEIGHT 0x0004
#define NVM_U32_LUB_DISTANCE 0x0005
#define NVM_U8_AGE 0x0006

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
#define DEFAULT_LUB_DISTANCE (1500)
#define DEFAULT_AGE (30)

typedef enum motor_communication_type {
  MOTOR_COMMUNICATION_TYPE_FREQ = 0,
  MOTOR_COMMUNICATION_TYPE_UART = 1,
} motor_communication_type_t;

typedef enum uart_protocol_state {
  UART_PROTOCOL_INIT_STATE = 0,
  UART_PROTOCOL_DETECTION_STATE,
  UART_PROTOCOL_PROCESS_MODBUS_STATE,
  UART_PROTOCOL_PROCESS_MOVEMENT_STATE,
  UART_PROTOCOL_PROCESS_UNKNOWN,
} uart_protocol_state_t;

typedef enum treadmill_scene {
  SCENE_INIT = 0,
  SCENE_IDLE,
  SCENE_LUB,
  SCENE_CONFIG,
  SCENE_PROGRAM,
  SCENE_COUNTDOWN,
  SCENE_TRAINING,
  SCENE_PAUSED,
  SCENE_REPORT,
  SCENE_TOTALS,
} treadmill_scene_t;

extern char *scene_name[];
typedef struct treadmill {
  treadmill_scene_t scene;
  treadmill_scene_t previous_scene;

  uint32_t scene_tick; // tick since scene started
  uint32_t idle_tick; // tick since last keypress

  struct program {
    program_id_t id;
    struct p_time p_time;
    struct p_distance p_distance;
    struct p_calories p_calories;
    struct p_hiit p_hiit;
    struct p_walk_30min p_walk_30min;
    struct p_fat_burn p_fat_burn;
    struct p_super_cardio p_super_cardio;
    uint8_t param_index;
  } program;

  struct training {
    uint32_t ms;
    uint32_t seconds;

    uint32_t cm;
    uint32_t meters;
    uint32_t kcal;

    uint16_t speed;
    int16_t incline;

    uint16_t pace;
    uint8_t bpm;

  } training;

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
    int lub_distance;
    int modbus;
    int test;

    char nsu[22];
    char nsp[22];  // legado
    char nsi[22];  // legado
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
  } countdown;

  struct debug {
    uint8_t print_states;
  } debug;

  struct hw {
    void (*beep)(beep_t beep);
    void (*update_rtc)(const uint32_t ms);
    motor_communication_type_t communication_type;
    uart_protocol_state_t protocol_state;

    ble_t ble;

    struct motor {
      int (*send)(const uint8_t *data, int length);
      int (*recv)(uint8_t *data, int max_length);
    } motor;

  } hw;



  uint32_t (*get_time_ms)(void);
  uint32_t ms;

} treadmill_t;

void treadmill_setup(treadmill_t *treadmill);

void treadmill_key_press(treadmill_t *treadmill, const kbd_key_t key);
void treadmill_update(treadmill_t *treadmill);

void treadmill_update(treadmill_t *treadmill);
void treadmill_set_scene(treadmill_t *treadmill, treadmill_scene_t scene);

uint32_t treadmill_calculate_pace(treadmill_t *treadmill);
uint32_t treadmill_calculate_calories(treadmill_t *treadmill);

void treadmill_load_config(treadmill_t *treadmill);
void treadmill_save_config(treadmill_t *treadmill);

#endif
