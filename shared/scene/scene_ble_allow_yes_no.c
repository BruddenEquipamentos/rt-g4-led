#include "../treadmill_motor_modbus.h"
#include "scene.h"
#include "../shared/dm/dm.h"
#include <string.h>

// #include "treadmill_ble.h" //Lucas
#include "../shared/ble/ble_protocol/ble_protocol_command.h" //Lucas
#include "../shared/treadmill_action.h" //Lucas

uint8_t ble_allowed = 0; //Lucas
 
static void keypress(treadmill_t* treadmill, const kbd_key_t key) {
   switch (key) {
       case KEY_START:{
           if (strlen(treadmill->hw.ble.hrm.device_name) >= 0){ ///////WIP - polar
               uint8_t buffer[128];
               uint8_t length;
               build_hrm_connect_packet(buffer, &length, 128);
               ble_post_message(buffer, "", 0);
               treadmill->hw.ble.status = BLE_STATUS_PERMISSION_ALLOWED;
               treadmill_set_scene(treadmill, SCENE_IDLE);
               ble_allowed = 1;
           }
           else{

           }
            break;
       }
       case KEY_STOP:
       case KEY_EXT_KBD_STOP:{
           uint8_t buffer[128];
           uint8_t length;
           build_hrm_disconnect_packet(buffer, &length, 128);
           ble_post_message(buffer, "", 0);
           treadmill->hw.ble.status = BLE_STATUS_PERMISSION_DENIED;
           treadmill->hw.ble.hrm.status = HRM_STATUS_DISCONNECTED; //Lucas
           memset(&treadmill->hw.ble.hrm, 0, sizeof(treadmill->hw.ble.hrm)); //Lucas -- disconnect from BLE strap only
           // TODO: Disconnect
           dm_clear_buffer();
           treadmill_set_scene(treadmill, SCENE_IDLE);
           ble_allowed = 0;
           break;
       }
       default:;
   }
}

static void update(treadmill_t* treadmill) {

    // if (treadmill->get_time_ms() - treadmill->scene_tick >= 2 * 1000) {
    //   treadmill_set_scene(treadmill, SCENE_IDLE);
    //   treadmill->scene_tick = treadmill->get_time_ms();
    // }
    if ((treadmill->hw.ble.status != BLE_STATUS_PERMISSION_ALLOWED) && (treadmill->get_time_ms() - treadmill->scene_tick >= 15 * 1000)) {
        treadmill_set_scene(treadmill, SCENE_IDLE);
        ble_allowed = 0;
    }

}

const scene_t scene_ble_allow_yes_no = { .id = SCENE_BLE_ALLOW_YES_NO, .name = "SCENE_BLE_ALLOW_Y_N", .keypress = keypress, .update = update };
