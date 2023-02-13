#include "treadmill_motor_modbus.h"

#include "modbus/mb_master.h"
#include "modbus/mb_operation.h"

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

int treadmill_motor_protocol_is_modbus(treadmill_t *treadmill) {
  enum mb_master_process status = MB_MASTER_OK;
  for (int i = 0; i < 6; ++i) {
    status = mb_operation_process(p023);
    if (status == MB_MASTER_OK) return 1;
  }
  return 0;
}

int treadmill_motor_modbus_init(void) {
  (void)bytes_4;
  p023 = mb_operation_get(mb_operations, mb_operations_length, 23);
  p682 = mb_operation_get(mb_operations, mb_operations_length, 682);
  p683 = mb_operation_get(mb_operations, mb_operations_length, 683);
  p856 = mb_operation_get(mb_operations, mb_operations_length, 856);
  p1000_1001 = mb_operation_get(mb_operations, mb_operations_length, 1000);
  return 1;
}

treadmill_motor_status_t treadmill_motor_modbus_process(
    treadmill_t *treadmill) {
  // if (pxVisSts->ucVelocidade > SPEED_MIN - 1) {
  //   p682->rval = 7;
  //   if (SPEED_MIN == 5) {
  //     p683->rval = modbusCalc(pxVisSts->ucVelocidade - (SPEED_MIN - 1));
  //   } else if (SPEED_MIN == 12) {
  //     p683->rval = modbusCalc(pxVisSts->ucVelocidade - (SPEED_MIN - 1) + 7);
  //   }
  // } else {
  //   p682->rval = 1;
  //   p683->rval = 0;
  // }

  return TREADMILL_MOTOR_STATUS_OK;
}
