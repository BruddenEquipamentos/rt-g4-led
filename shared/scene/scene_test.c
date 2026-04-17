#include "scene.h"
#include "treadmill_action.h"
#include "treadmill.h"
#include "../panel/rt_led.h"

static void keypress(treadmill_t* treadmill, const kbd_key_t key) {

	switch (key) {
	case KEY_START:		
		treadmill_start_training_countdown(treadmill);
		break;
	case KEY_STOP:		
	case KEY_EXT_KBD_STOP: 
		treadmill->config.test = RT_LED_CONFIG_TEST_DISABLE;
        treadmill_set_scene(treadmill, treadmill->previous_scene);
		break;
	case KEY_PROGRAM: {		
		break;
	}
	case KEY_CONFIG: {		
		break;
	}
	case KEY_LUB: {
		break;
	}
	case KEY_SAC: {
		break;
	}
	default:;
	}
}

static void update(treadmill_t* treadmill) {
	
}

const scene_t scene_test = { .id = SCENE_TEST, .name = "TEST", .keypress = keypress, .update = update };