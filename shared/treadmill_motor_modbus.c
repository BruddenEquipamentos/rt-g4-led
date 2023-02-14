#include "treadmill_motor_modbus.h"

#include "modbus/master_operations.h"
#include "modbus/mb_master.h"
#include "modbus/mb_operation.h"

// TEMP
extern treadmill_t *pTreadmill;

static const uint16_t pusVelocidadeModbusPos[] = {
    // 32 -> 0.5km/h, 70 -> 1.2km/h
    0,    32,   37,   43,   48,   54,   59,   65,   70,   76,   81,   87,
    92,   98,   103,  109,  114,  120,  125,  131,  136,  142,  147,  153,
    159,  164,  170,  175,  181,  186,  192,  197,  203,  208,  214,  219,
    225,  231,  236,  242,  247,  253,  258,  264,  269,  275,  280,  286,
    291,  297,  302,  308,  314,  319,  325,  330,  336,  341,  347,  352,
    358,  363,  369,  374,  380,  385,  391,  397,  402,  408,  413,  419,
    424,  430,  435,  441,  446,  452,  457,  463,  469,  474,  480,  485,
    491,  496,  502,  507,  513,  518,  524,  529,  535,  540,  546,  552,
    557,  563,  568,  574,  579,  585,  590,  596,  601,  607,  612,  618,
    624,  629,  635,  640,  646,  651,  657,  662,  668,  673,  679,  684,
    690,  695,  701,  707,  712,  718,  723,  729,  734,  740,  745,  751,
    756,  762,  767,  773,  778,  784,  790,  795,  801,  806,  812,  817,
    823,  828,  834,  839,  845,  850,  856,  862,  867,  873,  878,  884,
    889,  895,  900,  906,  911,  917,  922,  928,  933,  939,  945,  950,
    956,  961,  967,  972,  978,  983,  989,  994,  1000,  // 18
    1005, 1011, 1016, 1022, 1027, 1033, 1038, 1044, 1049, 1055, 1061, 1066,
    1072, 1077, 1083, 1088, 1094, 1099, 1105, 1110, 1116, 1121, 1127, 1132,
    1138, 1144, 1149, 1155, 1160, 1166, 1171, 1177, 1182, 1188, 1193, 1199,
    1204, 1210, 1215, 1221, 1227, 1232, 1238, 1243, 1249, 1254, 1260, 1265,
    1271, 1276, 1282, 1287, 1293, 1299, 1304, 1310, 1315, 1321, 1326, 1332,
    1337, 1343, 1348, 1354, 1359, 1365, 1370, 1376, 1382, 1387,
};

static uint8_t bytes_4[4] = {0};

static mb_operation_t *p023 = 0;
static mb_operation_t *p682 = 0;
static mb_operation_t *p683 = 0;
static mb_operation_t *p856 = 0;
static mb_operation_t *p1000_1001 = 0;

static uint16_t modbusCalc(uint16_t speed) {
  return (uint16_t)((pusVelocidadeModbusPos[speed] * 8192) / 600);
}

#include "debug.h"
#include "unistd.h"

static uint32_t modbus_rx_tick = 0;

static int modbus_master_send_message(mb_buffer_t *tx) {
  // DPRINT("MB TX: ");
  // for (int i = 0; i < tx->length; ++i) {
  //   DPRINT("%02X ", tx->data[i]);
  // }
  // DPRINT("\n");
  pTreadmill->hw.motor.send(tx->data, tx->length);
  modbus_rx_tick = pTreadmill->get_time_ms();
  return 1;
}

static int modbus_wait_for_recv_or_timeout(void) {
  if (!modbus_rx_tick) return 1;
  while (pTreadmill->get_time_ms() - modbus_rx_tick < 5) {
    usleep(1000 * 1);
  }
  modbus_rx_tick = 0;
  int length = pTreadmill->hw.motor.recv(mb_master_rx.data, 256);
  if (length > 0)
    mb_master_rx.length = length;
  else
    mb_master_rx.length = 0;

  DPRINT("MB RX: ");
  if (mb_master_rx.length > 0) {
    for (int i = 0; i < mb_master_rx.length; ++i) {
      DPRINT("%02X ", mb_master_rx.data[i]);
    }
  }
  DPRINT("\n");

  // // we must have receied a char at least in 100ms
  // if (xSemaphoreTake(xSemaphoreRx, 100) != pdTRUE) {
  //   modbus_rx_tick = 0;
  //   return 1;
  // }

  // // modbus tick must be setted to a value greater than zero
  // if (!modbus_rx_tick) return 1;

  // // if 5ms has passed since last char, process
  // portTickType xLastWakeTime;
  // while (((xLastWakeTime = uwTick) - modbus_rx_tick) < 10) {
  //   // while (((xLastWakeTime = xTaskGetTickCount()) - modbus_rx_tick) <  10)
  //   { vTaskDelayUntil(&xLastWakeTime, 1);
  // }

  // modbus_rx_tick = 0;
  // xSemaphoreTake(xSemaphoreRx, 0);  // todo(fminoru): test here

  // unsigned char ucData = 0;

  // while (xQueueReceive(xFifoVisRx, &ucData, 0) == pdPASS) {
  //   mb_master_push_rx(&ucData, 1);
  // }

  // return !(length > 0);  // timeout, 0 ok.
  return 0;
}

int treadmill_motor_modbus_init(void) {
  (void)bytes_4;
  mb_operations = mb_weg_ops;
  mb_operations_length = MB_WEG_NUM_OPS;
  mb_operations_display_length = sizeof(mb_ops_display) / sizeof(uint16_t);

  mb_master_send = modbus_master_send_message;
  mb_master_wait_recv_or_timeout = modbus_wait_for_recv_or_timeout;

  p023 = mb_operation_get(mb_operations, mb_operations_length, 23);
  p682 = mb_operation_get(mb_operations, mb_operations_length, 682);
  p683 = mb_operation_get(mb_operations, mb_operations_length, 683);
  p856 = mb_operation_get(mb_operations, mb_operations_length, 856);
  p1000_1001 = mb_operation_get(mb_operations, mb_operations_length, 1000);
  return 1;
}

int treadmill_motor_protocol_is_modbus(treadmill_t *treadmill) {
  enum mb_master_process status = MB_MASTER_OK;
  for (int i = 0; i < 6; ++i) {
    status = mb_operation_process(p023);
    if (status == MB_MASTER_OK) return 1;
  }
  return 0;
}

treadmill_motor_status_t treadmill_motor_modbus_process(
    treadmill_t *treadmill) {
  if (pTreadmill->training.speed > pTreadmill->config.speed.min - 10) {
    p682->rval = 7;
    if (pTreadmill->config.speed.min == 50) {
      p683->rval = modbusCalc((pTreadmill->training.speed - (40 - 10)) / 10);
    } else if (pTreadmill->config.speed.min == 120) {
      p683->rval =
          modbusCalc((pTreadmill->training.speed - (120 - 10) + 70) / 10);
    }
  } else {
    p682->rval = 1;
    p683->rval = 0;
  }

  p856->rval = pTreadmill->training.incline/10;

  mb_operation_t *op = mb_operation_fetch_next_op();

  // if (op->raddr == 1000) {
  //   uint32_t secs = DrvRTC_GetSeconds();
  //   p1000_1001->bytes[0] = (uint8_t)(secs >> 24);
  //   p1000_1001->bytes[1] = (uint8_t)(secs >> 16);
  //   p1000_1001->bytes[2] = (uint8_t)(secs >> 8);
  //   p1000_1001->bytes[3] = (uint8_t)(secs & 0xff);
  // }

  enum mb_master_process status = mb_operation_process(op);
  if (status == MB_MASTER_OK && op->iaddr == 1010) {
    // if (op->rval > 1) {
    //   status = modbus_collect_data(op->rval);
    // }
  }

  return TREADMILL_MOTOR_STATUS_OK;
}
