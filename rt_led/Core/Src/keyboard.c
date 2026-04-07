#include "main.h"
#include "stm32g070xx.h"
#include "cmsis_os.h"
#include "keyboard.h"
#include "treadmill.h"

#define ROW0 TS00_GPIO_Port, TS00_Pin
#define ROW1 TS01_GPIO_Port, TS01_Pin
#define ROW2 TS02_GPIO_Port, TS02_Pin
#define ROW3 TS03_GPIO_Port, TS03_Pin
#define ROW4 TS04_GPIO_Port, TS04_Pin
#define ROW5 TS05_GPIO_Port, TS05_Pin
#define ROW6 TS10_GPIO_Port, TS10_Pin
#define ROW7 TS11_GPIO_Port, TS11_Pin

#define COL0 TS06_GPIO_Port, TS06_Pin
#define COL1 TS07_GPIO_Port, TS07_Pin
#define COL2 TS08_GPIO_Port, TS08_Pin

#define COL3 TS09_GPIO_Port, TS09_Pin

extern treadmill_t *pTreadmill;

static int treadmill_has_incline(treadmill_t *treadmill) {
	return treadmill->config.incline.step > 0;
}

static void set_columns_to_high(void) {
  HAL_GPIO_WritePin(COL0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(COL3, GPIO_PIN_SET);
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

    if (treadmill_has_incline(pTreadmill)) {
        READ_KEYS_STATE(kPtr, COL2, ROW2, KEY_STOP);      // 1 -- ok
        READ_KEYS_STATE(kPtr, COL2, ROW0, KEY_SPEED_DECREMENT); // 2 -- ok
        READ_KEYS_STATE(kPtr, COL1, ROW0, KEY_SPEED_6);   // 3 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW0, KEY_SPEED_9);   // 4 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW1, KEY_SPEED_12)   // 5 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW2, KEY_SPEED_15);  // 6 -- ok
        READ_KEYS_STATE(kPtr, COL2, ROW1, KEY_SPEED_INCREMENT); // 7 -- ok
        READ_KEYS_STATE(kPtr, COL3, ROW1, KEY_START);     // 8 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW6, KEY_PROGRAM);   // 9 -- ok
        READ_KEYS_STATE(kPtr, COL1, ROW2, KEY_INCLINE_DECREMENT); // 10 -- ok
        READ_KEYS_STATE(kPtr, COL1, ROW1, KEY_INCLINE_INCREMENT); // 11 -- ok
        // READ_KEYS_STATE(kPtr, COL4, ROW3, KEY_INCLINE_3);   // 12 --
        READ_KEYS_STATE(kPtr, COL1, ROW6, KEY_INCLINE_6);   // 13 -- ok
        READ_KEYS_STATE(kPtr, COL2, ROW6, KEY_INCLINE_9);   // 14 -- ok
        READ_KEYS_STATE(kPtr, COL3, ROW6, KEY_INCLINE_12);  // 15 -- ok
        // READ_KEYS_STATE(kPtr, COL1, ROW1, KEY_SAC);         // 16
    } else { // CHECK
        READ_KEYS_STATE(kPtr, COL2, ROW2, KEY_STOP);      // 1 -- ok
        // READ_KEYS_STATE(kPtr, COL2, ROW0, KEY_SPEED_DECREMENT); // 2 -- ok
        READ_KEYS_STATE(kPtr, COL1, ROW0, KEY_SPEED_6);   // 3 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW0, KEY_SPEED_9);   // 4 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW1, KEY_SPEED_12)   // 5 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW2, KEY_SPEED_15);  // 6 -- ok
        READ_KEYS_STATE(kPtr, COL2, ROW1, KEY_SPEED_INCREMENT); // 7 -- ok
        READ_KEYS_STATE(kPtr, COL3, ROW1, KEY_START);     // 8 -- ok
        READ_KEYS_STATE(kPtr, COL0, ROW6, KEY_PROGRAM);   // 9 -- ok
        // READ_KEYS_STATE(kPtr, COL1, ROW2, KEY_INCLINE_DECREMENT); // 10 -- ok
        READ_KEYS_STATE(kPtr, COL1, ROW1, KEY_SPEED_DECREMENT); // 11 -- ok
        // READ_KEYS_STATE(kPtr, COL4, ROW3, KEY_INCLINE_3);   // 12 --
        READ_KEYS_STATE(kPtr, COL1, ROW6, KEY_INCLINE_6);   // 13 -- ok
        READ_KEYS_STATE(kPtr, COL2, ROW6, KEY_INCLINE_9);   // 14 -- ok
        READ_KEYS_STATE(kPtr, COL3, ROW6, KEY_INCLINE_12);  // 15 -- ok
    }


  }

  for (int i = 0; i < NUM_READS;++i) {
    key &= keys[i];
  }

  if (key & KEY_PROGRAM && pTreadmill->scene == SCENE_IDLE) {
      return KEY_NONE;
  }

  if (key & KEY_START && key & KEY_STOP) { //Lucas rework that 15/05 - wait for seconds see tft firmware 
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
