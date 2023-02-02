#include "main.h"
#include "stm32g070xx.h"
#include "cmsis_os.h"
#include "keyboard.h"

#define ROW0 TS00_GPIO_Port, TS00_Pin
#define ROW1 TS01_GPIO_Port, TS01_Pin
#define ROW2 TS02_GPIO_Port, TS02_Pin
#define ROW3 TS03_GPIO_Port, TS03_Pin
#define ROW4 TS04_GPIO_Port, TS04_Pin
#define ROW5 TS05_GPIO_Port, TS05_Pin

#define COL0 TS06_GPIO_Port, TS06_Pin
#define COL1 TS07_GPIO_Port, TS07_Pin
#define COL2 TS08_GPIO_Port, TS08_Pin

#define COL3 TS09_GPIO_Port, TS09_Pin
#define COL4 TS10_GPIO_Port, TS10_Pin
#define COL5 TS11_GPIO_Port, TS11_Pin

static void set_columns_to_high(void) {
  HAL_GPIO_WritePin(COL0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL4, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL5, GPIO_PIN_SET);
}

#define READ_KEYS_STATE(ptr_keys, col, row, key) \
  HAL_GPIO_WritePin(col, GPIO_PIN_RESET);        \
  if (HAL_GPIO_ReadPin(row) != GPIO_PIN_RESET)   \
    *ptr_keys = (*ptr_keys & ~key);              \
  else                                           \
    *ptr_keys = (*ptr_keys | key);               \
  set_columns_to_high();                         \
  __asm("nop\n");__asm("nop\n");

#define NUM_READS 4
enum key  get_key_pressed(void) {
  enum key keys[NUM_READS] = {0};
  enum key *kPtr = NULL;
  enum key key = 0xffff;

  for (int i = 0; i < NUM_READS; ++i) {
    kPtr = &keys[i];                                  // 16 ms
    READ_KEYS_STATE(kPtr, COL4, ROW5, KEY_STOP);      // 1
    READ_KEYS_STATE(kPtr, COL2, ROW0, KEY_SPEED_DECREMENT); // 2
    READ_KEYS_STATE(kPtr, COL1, ROW0, KEY_SPEED_5);   // 3
    READ_KEYS_STATE(kPtr, COL0, ROW0, KEY_SPEED_8);   // 4
    READ_KEYS_STATE(kPtr, COL0, ROW1, KEY_SPEED_11)   // 5
    READ_KEYS_STATE(kPtr, COL0, ROW2, KEY_SPEED_14);  // 6
    READ_KEYS_STATE(kPtr, COL2, ROW1, KEY_SPEED_INCREMENT); // 7
    READ_KEYS_STATE(kPtr, COL1, ROW2, KEY_START);     // 8
    READ_KEYS_STATE(kPtr, COL4, ROW4, KEY_PROGRAM);   // 9
    READ_KEYS_STATE(kPtr, COL5, ROW4, KEY_INCLINE_DECREMENT); // 10
    READ_KEYS_STATE(kPtr, COL5, ROW3, KEY_INCLINE_INCREMENT); // 11
    READ_KEYS_STATE(kPtr, COL4, ROW3, KEY_INCLINE_3);   // 12
    READ_KEYS_STATE(kPtr, COL3, ROW3, KEY_INCLINE_6);   // 13
    READ_KEYS_STATE(kPtr, COL3, ROW4, KEY_INCLINE_9);   // 14
    READ_KEYS_STATE(kPtr, COL3, ROW5, KEY_INCLINE_12);  // 15
    READ_KEYS_STATE(kPtr, COL1, ROW1, KEY_SAC);         // 16
  }

  for (int i = 0; i < NUM_READS;++i) {
    key &= keys[i];
  }

  if (key & KEY_START && key & KEY_STOP) {
      key &= ~(KEY_START | KEY_STOP);
      key |= KEY_CONFIG;
  } else {
      key &= ~KEY_CONFIG;
  }

  if (key == (KEY_SPEED_INCREMENT|KEY_SPEED_DECREMENT)) {
      key &= ~(KEY_SPEED_INCREMENT|KEY_SPEED_DECREMENT);
      key |= KEY_LUB;
  } else {
      key &= ~KEY_LUB;
  }

  return key;
}
