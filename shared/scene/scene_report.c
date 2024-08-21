#include "scene.h"

#include "treadmill_action.h" ///////

extern int report_state;
extern int completed_goal;
extern int report_goal;

static void keypress(treadmill_t *treadmill, const kbd_key_t key) {
  switch (key) {
    case KEY_START:
      report_goal = 0;
      treadmill_close_report(treadmill);
      break;
    case KEY_EXT_KBD_STOP:
      dm_reset_frame(); //////////
      report_goal = 0;
      treadmill_close_report(treadmill);
      break;
    default:;
  }
}
/// new update - alternate between avg and max values /////////////////
static void update(treadmill_t *treadmill) {

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
    treadmill_close_report(treadmill);
  }
  
}


scene_t scene_report = {
    .id = SCENE_REPORT,
    .name = "REPORT",
    .keypress = keypress,
    .update = update
};
