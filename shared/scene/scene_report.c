#include "scene.h"

#include "treadmill_action.h" ///////

extern int report_state;
extern int completed_goal;
extern int report_goal;

static uint32_t nfc_tick_ms = 0; // temp

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

      rt_led_send_training_data(treadmill); ///////////////////////////

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
  } else if (nfc_tick_ms != 0xffffffff) {
    if (treadmill->get_time_ms() - nfc_tick_ms >= 1500) {
      ble_post_message(
        "\x02"
        "NFC,0\n",
        "AOK", 1000);
      nfc_tick_ms = 0xffffffff;
    }
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
