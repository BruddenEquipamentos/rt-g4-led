#ifndef DESKTOP_MAIN_H
#define DESKTOP_MAIN_H

#include <stdint.h>

#include "../shared/treadmill_key.h"

typedef enum {
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET = 1,
} GPIO_PinState;

typedef struct GPIO_TypeDef GPIO_TypeDef;

#define EMG_GPIO_Port ((GPIO_TypeDef *)0)
#define EMG_Pin ((uint16_t)0)

static inline GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin) {
  (void)port;
  (void)pin;
  return GPIO_PIN_RESET;
}

#endif
