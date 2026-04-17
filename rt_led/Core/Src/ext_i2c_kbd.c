#include "ext_i2c_kbd.h"

#include "../shared/treadmill.h"
#include "main.h"

#include <string.h>

extern I2C_HandleTypeDef hi2c2;
extern treadmill_t *pTreadmill;

uint8_t _keys_debug[32] = {0};

int gstate = 0;
int dcount = 0;

enum ext_kbd_state {
    EXT_KBD_STATE_INIT = 0,
    EXT_KBD_STATE_IDLE,
    EXT_KBD_STATE_READ,
    EXT_KBD_STATE_PRESSED,
    EXT_KBD_STATE_REPEAT,
};

volatile enum ext_kbd_state ext_kbd_state = EXT_KBD_STATE_INIT;

void ext_i2c_kbd_irq(void) {
	dcount++;
    if (ext_kbd_state == EXT_KBD_STATE_IDLE) {
        ext_kbd_state = EXT_KBD_STATE_READ;
    }
}

void handle_key(uint8_t key);

void ext_i2c_kbd_handler(void) {
    static uint32_t kbd_state_idle_tick = 0;
    static uint32_t state_tick = 0;
    static uint32_t repeat_tick = 0;
    static uint32_t repeat_presses_count = 0;
    static uint32_t etick = 0;    
    
    gstate = ext_kbd_state;

    switch (ext_kbd_state) {
    case EXT_KBD_STATE_INIT: {
        HAL_GPIO_WritePin(RESET_N_GPIO_Port, RESET_N_Pin, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(RESET_N_GPIO_Port, RESET_N_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
        MX_I2C2_Init();
        ext_kbd_state = EXT_KBD_STATE_IDLE;
        kbd_state_idle_tick = HAL_GetTick();
        break;
    }

    case EXT_KBD_STATE_READ: {
        //
        static uint8_t cmd_read = 'R';
        static uint8_t read_buffer[8];
        if (HAL_I2C_Master_Transmit(&hi2c2, 0x18 << 1, &cmd_read, 1, 5) != HAL_OK) {
            ext_kbd_state = EXT_KBD_STATE_INIT;
            break;
        }

        if (HAL_I2C_Master_Receive(&hi2c2, 0x18 << 1, &read_buffer[0], 1, 5) != HAL_OK) {
            ext_kbd_state = EXT_KBD_STATE_INIT;
            break;
        }

    //    memmove(&_keys_debug[0], &_keys_debug[1], 31);
    //    _keys_debug[31] = read_buffer[0];

        handle_key(read_buffer[0]);

        ext_kbd_state = EXT_KBD_STATE_PRESSED;
        state_tick = HAL_GetTick();
        repeat_tick = HAL_GetTick();
        etick = HAL_GetTick();
        repeat_presses_count = 0;

        break;
    }

    case EXT_KBD_STATE_PRESSED: {
        if (HAL_GetTick() - state_tick >= 20) {
            static uint8_t cmd_read = 'R';
            static uint8_t read_buffer[8];
            if (HAL_I2C_Master_Transmit(&hi2c2, 0x18 << 1, &cmd_read, 1, 5) != HAL_OK) {
                ext_kbd_state = EXT_KBD_STATE_INIT;
                break;
            }

            if (HAL_I2C_Master_Receive(&hi2c2, 0x18 << 1, &read_buffer[0], 1, 5) != HAL_OK) {
                ext_kbd_state = EXT_KBD_STATE_INIT;
                break;
            }

            state_tick = HAL_GetTick();
            // memmove(&_keys_debug[0], &_keys_debug[1], 31);
            // _keys_debug[31] = read_buffer[0];

            if (read_buffer[0] == 0xff) {
                ext_kbd_state = EXT_KBD_STATE_IDLE;
                state_tick = HAL_GetTick();
                repeat_tick = 0;
                repeat_presses_count = 0;
                handle_key(read_buffer[0]); ///
                return;
            }

            // TODO: refactor
            uint32_t interval = 300;
            if (pTreadmill->config.incline.step > 0 && read_buffer[0] == 5 || read_buffer[0] == 8) {
                // incline increment / decrement
                if (repeat_presses_count < 3) {
                    interval = 150;
                } else {
                    interval = 50;
                }
            } else if (pTreadmill->config.incline.step  == 0 && read_buffer[0] == 5 || read_buffer[0] == 6) {
				if (pTreadmill->scene == SCENE_PROGRAM){ /////////// ok
					// speed increment / decrement
					if (repeat_presses_count < 3) {
						interval = 150;
					} else {
						interval = 50;
					}
				}
				else{
					ext_kbd_state = EXT_KBD_STATE_IDLE;
				}
            } else if (read_buffer[0] == 3 || read_buffer[0] == 6) {
				if (pTreadmill->scene == SCENE_PROGRAM){ /////////// ok
					// speed increment / decrement
					if (repeat_presses_count < 3) {
						interval = 150;
					} else {
						interval = 50;
					}
				// todo: limit the increment time window
				}
				else{
					ext_kbd_state = EXT_KBD_STATE_IDLE;
				}
            }

            if (HAL_GetTick() - repeat_tick >= interval) {
                static uint32_t etick = 0;
                static uint32_t delta = 0;
                repeat_tick = HAL_GetTick();
                repeat_presses_count++;
                handle_key(read_buffer[0]);
            }
        }
        break;
    }
    case EXT_KBD_STATE_IDLE: {
        // TODO: implement polling to check if Communication is alive
        uint8_t cmd_version = 'V';
        uint8_t buffer[8] = {0};

        if (HAL_GetTick() - state_tick >= 200) {
        	state_tick = HAL_GetTick();
        	if (HAL_I2C_Master_Transmit(&hi2c2, 0x18 << 1, &cmd_version, 1, 5) != HAL_OK) {
				ext_kbd_state = EXT_KBD_STATE_INIT;
				break;
			}

			if (HAL_I2C_Master_Receive(&hi2c2, 0x18 << 1, &buffer[0], 1, 5) != HAL_OK) {
				ext_kbd_state = EXT_KBD_STATE_INIT;
				break;
			}
        }
        break;
    }

    case EXT_KBD_STATE_REPEAT: {
        break;
    }
    }

    //	handle_key(0xff);
}

void handle_key(uint8_t key) {

	static uint32_t key_config_debounce = 0;
    static uint32_t key_ble_debounce = 0;
    static uint8_t _key;

    if (key == 0xff) {    	
        if (_key == 0x0B && key_ble_debounce != 0) {
            treadmill_key_press(pTreadmill, KEY_PROGRAM);
            _key = key;
        }
        key_config_debounce = 0;
        key_ble_debounce = 0;        
        return;
    }

    switch (key) {
    case 1: {
        treadmill_key_press(pTreadmill, KEY_EXT_KBD_SPEED_9);
        break;
    }
    case 2: {
        treadmill_key_press(pTreadmill, KEY_EXT_KBD_SPEED_6);
        break;
    }
    case 3: {        
        treadmill_key_press(pTreadmill, KEY_SPEED_DECREMENT);
        break;
    }
    case 4: {
        treadmill_key_press(pTreadmill, KEY_EXT_KBD_SPEED_12);
        break;
    }
    case 5: {
    	if (pTreadmill->config.incline.step == 0) {
    		treadmill_key_press(pTreadmill, KEY_SPEED_DECREMENT);
    	} else {
    		treadmill_key_press(pTreadmill, KEY_INCLINE_INCREMENT);
    	}

        break;
    }
    case 6: {
        treadmill_key_press(pTreadmill, KEY_SPEED_INCREMENT);
        break;
    }
    case 7: {
        treadmill_key_press(pTreadmill, KEY_EXT_KBD_SPEED_15);
        break;
    }
    case 8: {
        treadmill_key_press(pTreadmill, KEY_INCLINE_DECREMENT);
        break;
    }
    case 9: {
        treadmill_key_press(pTreadmill, KEY_EXT_KBD_STOP);
        break;
    }
    case 0x0A: {
        treadmill_key_press(pTreadmill, KEY_START);
        break;
    }
    case 0x0B: {
        if (pTreadmill->scene == SCENE_IDLE){
            if (key_ble_debounce == 0) {
                key_ble_debounce = pTreadmill->get_time_ms();
            } else {                
                if (pTreadmill->get_time_ms() - key_ble_debounce >= 1200 && pTreadmill->scene != SCENE_BLE_ALLOW_YES_NO) {
                    memset(pTreadmill->hw.ble.hrm.device_name, 0, sizeof(pTreadmill->hw.ble.hrm.device_name));
                    uint8_t buffer[64];
                    uint8_t length;
                    build_hrm_scan_packet(buffer, &length, 64);
                    ble_post_message(buffer, "", 0);
                    treadmill_set_scene(pTreadmill, SCENE_BLE_ALLOW_YES_NO);
                    key_ble_debounce = 0;
                }
            }            
        } else{
            treadmill_key_press(pTreadmill, KEY_PROGRAM);
        }
        break;
    }
    case 0x0C: {
        treadmill_key_press(pTreadmill, KEY_INCLINE_6);
        break;
    }
    case 0x0D: {
        treadmill_key_press(pTreadmill, KEY_INCLINE_9);
        break;
    }
    case 0x0E: {
        treadmill_key_press(pTreadmill, KEY_INCLINE_12);
        break;
    }
    case 0x0F: {
        // treadmill_key_press(pTreadmill, KEY_CONFIG);
		if (key_config_debounce == 0) {
			key_config_debounce = pTreadmill->get_time_ms();
		} else {
			if (pTreadmill->get_time_ms() - key_config_debounce >= 1200) {
                if (pTreadmill->scene != SCENE_CONFIG){
                    treadmill_key_press(pTreadmill, KEY_CONFIG);
                    key_config_debounce = 0;
                }
                if (pTreadmill->scene == SCENE_CONFIG && (pTreadmill->get_time_ms() - pTreadmill->scene_tick > 1000)){
                    treadmill_key_press(pTreadmill, KEY_CONFIG);
                    key_config_debounce = 0;
                }
			}
		}
        break;
    }
    case 0x10: {
        treadmill_key_press(pTreadmill, KEY_PROGRAM);
        break;
    }
    default:
        break;
    }

    _key = key;
}