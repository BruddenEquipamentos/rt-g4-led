#include "scene.h"
#include "../shared/dm/dm.h"
#include "../panel/rt_led.h"
#include "treadmill_action.h" ///////

extern int report_state;
extern int completed_goal;
extern int report_goal;

static uint32_t nfc_tick_ms = 0; // temp
static uint32_t training_data_last_attempt_ms = 0U;
static uint8_t training_data_attempts = 0U;
static uint8_t training_data_queued = 0U;

#define TRAINING_DATA_MAX_ATTEMPTS 3U
#define TRAINING_DATA_RETRY_MS 500U

static void reset_training_data_queue(void) {
  training_data_last_attempt_ms = 0U;
  training_data_attempts = 0U;
  training_data_queued = 0U;
}

static void queue_training_data_once(treadmill_t *treadmill) {
  uint32_t now = treadmill->get_time_ms();

  if (training_data_queued || training_data_attempts >= TRAINING_DATA_MAX_ATTEMPTS) {
    return;
  }
  if (treadmill->hw.ble.state != BLE_STATE_READY ||
      !treadmill->hw.ble.connected ||
      !treadmill->hw.ble.ftms.subscribed) {
    return;
  }
  if (training_data_attempts > 0U &&
      now - training_data_last_attempt_ms < TRAINING_DATA_RETRY_MS) {
    return;
  }

  training_data_last_attempt_ms = now;
  ++training_data_attempts;
  if (rt_led_send_training_data(treadmill)) {
    training_data_queued = 1U;
  }
}

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
    case KEY_EXT_KBD_START:
      report_goal = 0;
      // Enable NFC on report screen (TODO: only for Samsung)
      ble_post_message(
          "\x02"
          "NFC,1\n",
          "AOK", 1000);

      treadmill_close_report(treadmill);
      nfc_tick_ms = 0; // temp
      break;
    case KEY_EXT_KBD_STOP:
    case KEY_STOP:
      dm_reset_frame(); //////////
      report_goal = 0;
      // Enable NFC on report screen (TODO: only for Samsung)
      ble_post_message(
          "\x02"
          "NFC,1\n",
          "AOK", 1000);

      treadmill_close_report(treadmill);
      nfc_tick_ms = 0; // temp
      break;
    default:;
  }
}

/// new update - alternate between avg and max values /////////////////
static void update(treadmill_t *treadmill) {

  // temp
  if (nfc_tick_ms == 0) {
    nfc_tick_ms = treadmill->get_time_ms();
    reset_training_data_queue();
  } else if (nfc_tick_ms != 0xffffffffU) {
    if (treadmill->get_time_ms() - nfc_tick_ms >= 1500) {
      if (ble_post_message(
            "\x02"
            "NFC,0\n",
            "AOK", 1000)) {
        nfc_tick_ms = 0xffffffffU;
      }
    }
  }

  /* BLE commands are processed in FIFO order. Queue the Movement report only
     after NFC,0 so its AOK is handled before the report is transmitted. */
  if (nfc_tick_ms == 0xffffffffU) {
    queue_training_data_once(treadmill);
  }
  
  if (completed_goal == 0){
    if (treadmill->get_time_ms() - treadmill->scene_tick >= 3 * 1000) {
      report_state = report_state == 0 ? 1 : 0;
      treadmill->scene_tick = treadmill->get_time_ms();
    }
  }
  else if (completed_goal == 1) {
    if ((report_goal == 1) && (treadmill->get_time_ms() - treadmill->scene_tick >= 3 * 1000)) {
      report_state = report_state == 0 ? 1 : 0;
      treadmill->scene_tick = treadmill->get_time_ms();
    }
  } 

  if (treadmill->get_time_ms() - treadmill->idle_tick >= 30 * 1000) {
    report_goal = 0;

    // Enable NFC on report screen (TODO: only for Samsung)
    ble_post_message(
        "\x02"
        "NFC,1\n",
        "AOK", 1000);
    treadmill_close_report(treadmill);
    nfc_tick_ms = 0; // temp
  }
  
}


scene_t scene_report = {
    .id = SCENE_REPORT,
    .name = "REPORT",
    .keypress = keypress,
    .update = update
};
