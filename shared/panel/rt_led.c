#include "rt_led.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>


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

char training_json[192];
static const char *treadmill_persisted_data =
       "{\"duration\":%u,\"dist\":%u,\"cal\":%u,\"avgSpd\":%u,\"maxSpd\":%u,"
       "\"avgPace\":%u,\"avgHR\":%u,\"maxHR\":%u}";

uint16_t fw_version = MAJOR*100 + MINOR*10 + REVISION;

static const scene_t *rt_led_scenes[] = {
    &scene_init,     &scene_idle,   &scene_config, &scene_lub,      &scene_countdown,
    &scene_training, &scene_paused, &scene_report, &scene_programs, &scene_emergency, &scene_ble_allow_yes_no,
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
  if (!(treadmill->scene == SCENE_IDLE && key == KEY_START) &&
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

  if (treadmill->config.device_name_id == 1){
    strncpy(treadmill->config.nsu, "5000000597694",22);
  }
  if (treadmill->config.device_name_id == 2){
    strncpy(treadmill->config.nsu, "5000000597692",22);
  }
  if (treadmill->config.device_name_id == 3){
    strncpy(treadmill->config.nsu, "5000000597693",22);
  }
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

// lucas testing 27/10
void rt_led_send_training_data(treadmill_t *treadmill) {
    // send persisted training data via BLE on entering report scene - static
    
    // static char datetime[32] = {0};
    // RTC_GetFormattedDatetime(datetime, sizeof(datetime));

    /* build JSON payload with the fields from struct training / totals */
    // char training_json[192] = {0};
    snprintf(training_json, 192, treadmill_persisted_data,
             (unsigned)treadmill->training.seconds,            // duration (s)
             (unsigned)treadmill->training.meters,             // distance (m)
             (unsigned)treadmill->training.kcal,               // calories
             (unsigned)treadmill->training.average_speed,      // avgSpeed
             (unsigned)treadmill->training.speed_max,          // maxSpeed
             (unsigned)treadmill->training.pace,               // avgPace
             (unsigned)treadmill->training.bpm_average,        // avgHR
             (unsigned)treadmill->training.bpm_max            // maxHR
             );                                        // timestamp (formatted) TODO

    /* set BLE flags then send the MD command with the JSON payload */
    ble_post_message("\x02"
                     "SET_BLE_FLAGS,32\n",
                     "", 1000);

    char data[192] = {0};
    snprintf(data, 192,
             "\x02"
             "MD,%s\n",
             training_json);

   ble_post_long_message(data, "AOK", 1000); // Lucas testing - just for MD command
}
