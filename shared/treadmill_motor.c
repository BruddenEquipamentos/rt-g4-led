#include "treadmill_motor.h"

#include "treadmill_motor_modbus.h"
#include "treadmill_motor_movement.h"

static void treadmill_motor_frequency_process(treadmill_t *treadmill, const uint16_t speed) {
  static const uint16_t speed_freq[] = {
      0,    20402, 18916, 17631, 16510, 15523, 14648, 13865, 13102, 12473, 11901, 11380, 10902, 10463, 10057, 9682,
      9334, 9010,  8708,  8426,  8161,  7912,  7658,  7438,  7231,  7035,  6850,  6674,  6506,  6347,  6196,  6051,
      5914, 5782,  5656,  5535,  5410,  5299,  5193,  5091,  4993,  4899,  4809,  4721,  4637,  4555,  4477,  4401,
      4328, 4257,  4188,  4116,  4051,  3989,  3929,  3870,  3813,  3758,  3705,  3653,  3602,  3553,  3505,  3458,
      3408, 3364,  3321,  3279,  3238,  3199,  3160,  3122,  3085,  3048,  3013,  2978,  2945,  2912,  2876,  2845,
      2814, 2784,  2754,  2725,  2697,  2669,  2642,  2616,  2590,  2564,  2539,  2514,  2488,  2464,  2441,  2419,
      2396, 2374,  2353,  2332,  2311,  2291,  2270,  2251,  2231,  2212,  2192,  2174,  2156,  2138,  2120,  2103,
      2086, 2070,  2053,  2037,  2021,  2006,  1990,  1975,  1959,  1944,  1930,  1916,  1902,  1888,  1874,  1861,
      1848, 1834,  1822,  1809,  1796,  1784,  1771,  1759,  1747,  1735,  1724,  1712,  1701,  1690,  1679,  1668,
      1658, 1647,  1637,  1627,  1616,  1606,  1596,  1586,  1576,  1567,  1557,  1548,  1539,  1530,  1521,  1512,
      1503, 1495,  1485,  1477,  1469,  1460,  1452,  1444,  1436,  1428,
  };

  if (speed >= treadmill->config.speed.min) {
    uint16_t index = (speed - (treadmill->config.speed.min - 10)) / 10;
    treadmill->hw.inverter.change_frequency(speed_freq[index]);
  } else {
    treadmill->hw.inverter.change_frequency(speed_freq[0]);
  }
}

static void treadmill_motor_uart_protocol_init(treadmill_t *treadmill) {
  treadmill_motor_modbus_init();
  treadmill->hw.inverter.protocol = UART_PROTOCOL_DETECTION_STATE;
}

static int treadmill_motor_uart_protocol_detection(treadmill_t *treadmill) {
  if (treadmill_motor_protocol_is_modbus(treadmill)) {
    treadmill->hw.inverter.protocol = UART_PROTOCOL_PROCESS_MODBUS_STATE;
    return 1;
  } else if (treadmill_motor_protocol_is_movement(treadmill)) {
    treadmill->hw.inverter.protocol = UART_PROTOCOL_PROCESS_MOVEMENT_STATE;
    return 1;
  }
  return 0;
}

static void treadmill_motor_uart_process(treadmill_t *treadmill) {
  static int protocol_detection_error_count = 0;
  static uint32_t init_tick = 0;

  switch (treadmill->hw.inverter.protocol) {
    case UART_PROTOCOL_INIT_STATE: {
      if (init_tick == 0) {
        init_tick = treadmill->get_time_ms();
      } else if (treadmill->get_time_ms() - init_tick >= 1000) {
        protocol_detection_error_count = 0;
        treadmill_motor_uart_protocol_init(treadmill);
      }
      break;
    }
    case UART_PROTOCOL_DETECTION_STATE: {
      if (!treadmill_motor_uart_protocol_detection(treadmill)) {
        protocol_detection_error_count++;
        if (protocol_detection_error_count > 20) {
          treadmill->hw.inverter.protocol = UART_PROTOCOL_PROCESS_UNKNOWN;
        }
      }
      break;
    }
    case UART_PROTOCOL_PROCESS_MODBUS_STATE: {
      treadmill_motor_modbus_process(treadmill);
      break;
    }
    case UART_PROTOCOL_PROCESS_UNKNOWN:
    case UART_PROTOCOL_PROCESS_MOVEMENT_STATE: {
      treadmill_motor_movement_process(treadmill);
      break;
    }
  }
}

// TODO: When changing model, reset params
void treadmill_motor_process(treadmill_t *treadmill) {
  switch (treadmill->hw.inverter.comm_type) {
    case MOTOR_COMMUNICATION_TYPE_FREQ: {
      treadmill_motor_frequency_process(treadmill, treadmill->training.speed);
      break;
    }
    case MOTOR_COMMUNICATION_TYPE_UART: {
      treadmill_motor_uart_process(treadmill);
      treadmill_motor_frequency_process(treadmill, treadmill->training.speed > 0 ? treadmill->config.speed.min : 0);
      break;
    }
    default:;
  }
}
