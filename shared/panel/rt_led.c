#include "rt_led.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

extern scene_t scene_init;
extern scene_t scene_idle;
extern scene_t scene_emergency;
extern scene_t scene_lub;
extern scene_t scene_config;
extern scene_t scene_paused;
extern scene_t scene_training;
extern scene_t scene_countdown;
extern scene_t scene_report;
extern scene_t scene_programs;
extern scene_t scene_ble_allow_yes_no;
extern scene_t scene_test;

uint16_t fw_version = MAJOR*100 + MINOR*10 + REVISION;

static const scene_t *rt_led_scenes[] = {
    &scene_init,     &scene_idle,   &scene_config, &scene_lub,      &scene_countdown,
    &scene_training, &scene_paused, &scene_report, &scene_programs, &scene_emergency, &scene_ble_allow_yes_no, &scene_test,
};

static const scene_t *rt_led_current_scene(treadmill_scene_t treadmill_scene) {
  for (uint32_t i = 0; i < sizeof(rt_led_scenes) / sizeof(scene_t *); ++i) {
    if (rt_led_scenes[i]->id == treadmill_scene) {
      return rt_led_scenes[i];
    }
  }
  return NULL;
}

void rt_led_key_press(treadmill_t *treadmill, const kbd_key_t key) {
  const scene_t *scene = rt_led_current_scene(treadmill->scene);
  if (scene == NULL) return;

  // prevent beeping on keypress if it is going to start the countdown
  if (!(treadmill->scene == SCENE_IDLE && (key == KEY_START || key == KEY_EXT_KBD_START)) &&
      !(treadmill->scene == SCENE_LUB && treadmill->key_pressed_tick > 0)) {
    treadmill->hw.beep(BEEP_KEYPRESS_ACK);
  }

  scene->keypress(treadmill, key);
}

void rt_led_update(treadmill_t *treadmill) {
  const scene_t *scene = rt_led_current_scene(treadmill->scene);
  if (scene == NULL) return;
  scene->update(treadmill);
}

void rt_led_render(treadmill_t *treadmill) {
  sd_render(treadmill);
  dm_render(treadmill);
}

extern uint8_t __new_display;

void rt_led_ble_name_update(treadmill_t *treadmill) {
  char device_name_id[15];  // espaço para o nome + número + null terminator
  if (treadmill->config.model == RT_LED_MODEL_150_FREQ || treadmill->config.model == RT_LED_MODEL_150_USART) {
    memcpy(device_name_id, "ARIA LED ", 9);  // Copia "ARIA LED " para device_name_id
    device_name_id[9] = '0' + (treadmill->config.device_name_id / 10);
    device_name_id[10] = '0' + (treadmill->config.device_name_id % 10);
    device_name_id[11] = '\0';
  } else if (treadmill->config.model == RT_LED_MODEL_250_USART) {
    memcpy(device_name_id, "ARIA iLED ", 10);  // Copia "ARIA iLED " para device_name_id
    device_name_id[10] = '0' + (treadmill->config.device_name_id / 10);
    device_name_id[11] = '0'+ (treadmill->config.device_name_id % 10);
    device_name_id[12] = '\0';
  } else {
    return; // Modelo desconhecido, não faz nada
  }
  strncpy(treadmill->config.device_name, device_name_id, 15);
}

int rt_led_send_training_data(treadmill_t *treadmill) {
  static const char command_format[] =
      "\x02"
      "MD,{\"duration\":%u,\"dist\":%u,\"cal\":%u,\"avgSpd\":%u,\"maxSpd\":%u,"
      "\"avgPace\":%u,\"avgHR\":%u,\"maxHR\":%u}\n";
  char command[192] = {0};

  if (treadmill == NULL ||
      treadmill->hw.ble.state != BLE_STATE_READY ||
      !treadmill->hw.ble.connected ||
      !treadmill->hw.ble.ftms.subscribed) {
    return 0;
  }

  /* Movement report contract: duration=s, dist=m, cal=kcal,
     speed=0.01 km/h, pace=s/km and HR=bpm. */
  uint32_t average_pace = treadmill->training.average_speed > 0U
                              ? 360000U / treadmill->training.average_speed
                              : 0U;
  int command_length = snprintf(command, sizeof(command), command_format,
                                (unsigned)treadmill->training.seconds,
                                (unsigned)treadmill->training.meters,
                                (unsigned)(treadmill->training.kcal / 1000U),
                                (unsigned)treadmill->training.average_speed,
                                (unsigned)treadmill->training.speed_max,
                                (unsigned)average_pace,
                                (unsigned)treadmill->training.bpm_average,
                                (unsigned)treadmill->training.bpm_max);
  if (command_length <= 0 || command_length >= (int)sizeof(command)) {
    return 0;
  }

  return ble_post_long_message(command, "AOK", 1000);
}

void rt_led_load_model_params(treadmill_t *treadmill, rt_led_model_t model) {
  treadmill->config.model = model;
  switch (model) {
    case RT_LED_MODEL_150_FREQ: {
      treadmill->config.incline.min = 0;
      treadmill->config.incline.max = 0;
      treadmill->config.incline.step = 0;

      treadmill->config.speed.min = 120;
      treadmill->config.speed.max = 1800;
      treadmill->config.speed.step = 10;

      treadmill->config.weight = 70;

      treadmill->config.language = RT_LED_CONFIG_LNG_PT_BR;

      treadmill->hw.inverter.comm_type = MOTOR_COMMUNICATION_TYPE_FREQ;
      
      rt_led_ble_name_update(treadmill);

      __new_display = 1;
      break;
    }
    case RT_LED_MODEL_150_USART: {
      treadmill->config.incline.min = 0;
      treadmill->config.incline.max = 0;
      treadmill->config.incline.step = 0;

      treadmill->config.speed.min = 120;
      treadmill->config.speed.max = 1800;
      treadmill->config.speed.step = 10;

      treadmill->config.weight = 70;

      treadmill->config.language = RT_LED_CONFIG_LNG_PT_BR;

      treadmill->hw.inverter.comm_type = MOTOR_COMMUNICATION_TYPE_UART;

      rt_led_ble_name_update(treadmill);

      __new_display = 1;
      break;
    }
    case RT_LED_MODEL_250_USART: {
      treadmill->config.incline.min = 0;
      treadmill->config.incline.max = 150;
      treadmill->config.incline.step = 10;

      treadmill->config.speed.min = 120;
      treadmill->config.speed.max = 1800;
      treadmill->config.speed.step = 10;

      treadmill->config.weight = 70;

      treadmill->config.language = RT_LED_CONFIG_LNG_PT_BR;

      treadmill->hw.inverter.comm_type = MOTOR_COMMUNICATION_TYPE_UART;

      rt_led_ble_name_update(treadmill);

      __new_display = 1;
      break;
    }
    default:;
  }
}
