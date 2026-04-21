#ifndef EXT_I2C_KBD_H
#define EXT_I2C_KBD_H

#include <stdint.h>

void ext_i2c_kbd_handler(void);
uint8_t ext_i2c_kbd_is_clear_combo_pressed(void);

#endif
