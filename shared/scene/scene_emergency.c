#include "../treadmill_motor_modbus.h"
#include "scene.h"
#include "main.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  treadmill->idle_tick = treadmill->get_time_ms();
}

static void update(treadmill_t *treadmill) {  
  if (treadmill->get_time_ms() - treadmill->scene_tick >= 5000 && HAL_GPIO_ReadPin(EMG_GPIO_Port, EMG_Pin) == GPIO_PIN_RESET) {
    treadmill_set_scene(treadmill, SCENE_IDLE);
  }
}

const scene_t scene_emergency = {
    .id = SCENE_EMERGENCY, .name = "EMERGENCY", .keypress = keypress, .update = update};
