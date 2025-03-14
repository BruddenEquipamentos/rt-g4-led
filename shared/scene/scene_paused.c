#include "scene.h"

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
    case KEY_EXT_KBD_START:
      treadmill_resume_training(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
    case KEY_STOP:
      dm_reset_frame(); ////////
      // Disable NFC on report screen (TODO: only for Samsung)
      ble_post_message(
          "\x02"
          "NFC,0\n",
          "AOK", 1000);
      treadmill_finish_training(treadmill);
      break;
    case KEY_SPEED_INCREMENT:
      break;
    case KEY_SPEED_DECREMENT:
      break;
    case KEY_INCLINE_INCREMENT:
      break;
    case KEY_INCLINE_DECREMENT:
      break;
    default:;
  }
}

static void update(treadmill_t *treadmill) {}

scene_t scene_paused = {
    .id = SCENE_PAUSED,
    .name = "PAUSED",
    .keypress = keypress,
    .update = update
};
