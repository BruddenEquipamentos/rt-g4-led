#include "treadmill_motor_movement.h"

uint32_t last_ack_time = 0;

int treadmill_motor_protocol_is_movement(treadmill_t *treadmill) {
  uint8_t recv_buffer[8];
  uint8_t cmd[] = {0x02, 0x41, 0x00, 0x02, 0x42, 0x00, 0x02, 0x43, 0x00};
  treadmill->hw.inverter.send_data(cmd, sizeof(cmd));
  int length = treadmill->hw.inverter.recv_data(recv_buffer, 8);
  return (length > 0 && recv_buffer[0] == 0x06);
}

treadmill_motor_status_t treadmill_motor_movement_process(
    treadmill_t *treadmill) {
  uint8_t cmd[] = {0x02, 0x41, 0x00, 0x02, 0x42, 0x00, 0x02, 0x43, 0x00};
  uint8_t recv_buffer[8];
  if (treadmill->training.speed >= treadmill->config.speed.min)
    cmd[2] =
        (treadmill->training.speed - (treadmill->config.speed.min - 10)) / 10;
  else
    cmd[2] = 0;

  cmd[5] = treadmill->training.incline / 10;

  cmd[8] = treadmill->config.weight;

  treadmill->hw.inverter.send_data(cmd, sizeof(cmd));

  int length = treadmill->hw.inverter.recv_data(recv_buffer, 8);

  if (length > 0) {
    if (recv_buffer[0] == 0x06) {
      last_ack_time = treadmill->get_time_ms();
      return TREADMILL_MOTOR_STATUS_OK;
    } else {
      // Resposta não-vazia diferente de 0x06: registrar como erro
      treadmill_motor_register_inverter_error(treadmill, recv_buffer[0]);
      return TREADMILL_MOTOR_STATUS_ERROR;
    }
  }    


  return TREADMILL_MOTOR_STATUS_ERROR;
}
