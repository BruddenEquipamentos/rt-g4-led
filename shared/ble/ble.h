#ifndef BLE_H
#define BLE_H

#include <stdint.h>

typedef enum ble_state {
  BLE_STATE_INIT = 0,
  BLE_STATE_READY,
  BLE_STATE_ERROR,
} ble_state_t;

typedef enum ble_init_state {
  BLE_INIT_STATE_INIT = 0,
  BLE_INIT_STATE_PROCESS,
  BLE_INIT_STATE_READY
} ble_init_state_t;

typedef enum ble_process_status {
  BLE_PROCESS_STATUS_ERROR = 0,
  BLE_PROCESS_STATUS_OK,
  BLE_PROCESS_STATUS_IDLE
} ble_process_status_t;

typedef int (*send_data_cb)(const uint8_t *data, int length);
typedef int (*recv_data_cb)(uint8_t *data, int max_length);

typedef struct ble {
  int connected;

  ble_state_t state;
  ble_init_state_t init_state;

  send_data_cb send_data;
  recv_data_cb recv_data;
  uint32_t (*get_tick)(void);

} ble_t;

// extern int (*ble_send_data)(char *data, int length) = NULL;
// extern int (*ble_read_data)(char *data, const int max_length) = NULL;

void ble_reset(ble_t *ble);
void ble_clear_commands(void);
int ble_post_command(const char *data, const char *response,
                     const uint32_t timeout_tick);

int ble_process_received(ble_t *ble, char *data, int length);
int ble_process_commands(ble_t *ble);

#endif
