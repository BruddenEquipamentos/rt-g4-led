#include "ble.h"

#include <string.h>

#include "../debug.h"
#include "./ble_protocol/ble_fmcp.h"
#include "./ble_protocol/ble_ftms_treadmill.h"
#include "./ble_protocol/ble_protocol_helper.h"
#include "ble_legacy.h"

// TEMP
#include "../treadmill.h"
#include "../treadmill_action.h"

#include <assert.h>

extern treadmill_t *pTreadmill;

typedef enum ble_command_status {
    BLE_COMMAND_STATUS_PENDING = 0,
    BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE,
    BLE_COMMAND_STATUS_TIMEDOUT,
    BLE_COMMAND_STATUS_OK,
    BLE_COMMAND_STATUS_ERROR,
} ble_command_status_t;

typedef struct ble_command {
    char data[32];
    char response[16];
    ble_command_status_t status;
    uint32_t tick;
    uint32_t timeout_tick;
    uint32_t delay;
} ble_command_t;

#define MAX_NUM_COMMANDS 8
static ble_command_t commands[MAX_NUM_COMMANDS];
static int head = 0;
static int tail = 0;

void ble_clear_commands(void) {
    memset(commands, 0x00, sizeof(commands));
    head = 0;
    tail = 0;
}

int ble_post_message(const char *data, const char *response, const uint32_t timeout_tick) {
    ble_command_t *cmd = &commands[head];
    strncpy(cmd->data, data, 32);
    strncpy(cmd->response, response, 16);
    cmd->tick = 0;
    cmd->timeout_tick = timeout_tick;
    cmd->status = BLE_COMMAND_STATUS_PENDING;
    head = (head + 1) % MAX_NUM_COMMANDS;
    return 1;
}

// static void temp_debug_print(const char *response, const int length) {
//     DPRINT("Recv: %s", response);
//     for (int i = 0; i < length; ++i) {
//         DPRINT("%02X ", response[i]);
//     }
//     DPRINT("\n");
// }

static int is_expected_response(ble_command_t *cmd, const char *response, const int length) {
    return strstr(response, cmd->response) != NULL;
}

static int is_error_response(ble_command_t *cmd, const char *response, const int length) {
    if (strcmp(cmd->response, "AOK") == 0) {
        if (strstr(response, "ERR")) {
            DPRINT("GOT ERROR\n");
            return 1;
        }
    }
    return 0;
}

static int is_timedout(ble_t *ble, ble_command_t *cmd) {
    if (ble->get_tick() - cmd->tick >= cmd->timeout_tick) {
        DPRINT("Timedout\n");
        return 1;
    }
    return 0;
}

static int ble_ftms_control_point_response(uint8_t response_opcode, uint8_t opcode, fmcp_result_t result) {
    uint8_t response[40]; // todo: predefine buffer
    uint8_t index = 0;
    uint8_t cs = 0;

    response[index++] = BLE_PROTOCOL_MESSAGE_START;
    flat_bin_to_ascii_8(response, &index, 40, BP_CONTROL_POINT);
    flat_bin_to_ascii_8(response, &index, 40, response_opcode);
    flat_bin_to_ascii_8(response, &index, 40, opcode);
    flat_bin_to_ascii_8(response, &index, 40, result);

    cs = ble_protocol_checksum(response, index);
    flat_bin_to_ascii_8(response, &index, 40, cs);

    response[index++] = BLE_PROTOCOL_MESSAGE_END;

    return pTreadmill->hw.ble.send_data(response, index);
}

static int ble_ftms_control_point(char *ptr) {
    fmcp_result_t result = FMCP_RESULT_SUCCESS;
    uint8_t opcode = next_u8(&ptr);
    switch (opcode) {
    case FMCP_REQUEST_CONTROL: {
        // todo: rule to allow control point control
        pTreadmill->hw.ble.ftms.control_point_allowed = 1;
        break;
    }
    case FMCP_RESET: {
        // todo: implement
        break;
    }
    case FMCP_SET_TARGET_SPEED: {
        uint16_t speed = next_u16_le(&ptr);
        pTreadmill->hw.beep(BEEP_KEYPRESS_ACK);
        if (!treadmill_set_speed(pTreadmill, speed))
            result = FMCP_RESULT_CODE_OPERATION_FAILED;
        break;
    }
    case FMCP_SET_TARGET_INCLINATION: {
        int16_t inclination = next_i16_le(&ptr);
        pTreadmill->hw.beep(BEEP_KEYPRESS_ACK);
        if (!treadmill_set_incline(pTreadmill, inclination))
            result = FMCP_RESULT_CODE_OPERATION_FAILED;
        break;
    }
    case FMCP_SET_TARGET_START_OR_RESUME: {
        if (pTreadmill->hw.ble.ftms.control_point_allowed) {
            if (pTreadmill->scene == SCENE_PAUSED)
                pTreadmill->hw.beep(BEEP_KEYPRESS_ACK);
            if (!treadmill_start_or_resume(pTreadmill))
                result = FMCP_RESULT_CODE_OPERATION_FAILED;
        } else {
            result = FMCP_RESULT_CODE_CONTROL_NOT_PERMITTED;
        }
        break;
    }
    case FMCP_SET_TARGET_STOP_OR_PAUSE: {
        uint8_t param = next_u8(&ptr);
        pTreadmill->hw.beep(BEEP_KEYPRESS_ACK);
        if (param == 1) {
            if (!treadmill_stop_procedure(pTreadmill))
                result = FMCP_RESULT_CODE_OPERATION_FAILED;
        } else if (param == 2) {
            if (!treadmill_pause_procedure(pTreadmill))
                result = FMCP_RESULT_CODE_OPERATION_FAILED;
        }
        break;
    }
    default:
        result = FMCP_RESULT_CODE_OPCODE_NOT_SUPPORTED;
        break;
    }

    return ble_ftms_control_point_response(FMCP_RESPONSE_CODE, opcode, result);
}

char debug_string[128] = {0};

void ble_protocol_process(uint8_t *ptr_data, int length) {
    if (!ble_protocol_valid_message(ptr_data, length)) {
        return;
    }

    char *ptr = (char *)&ptr_data[1];
    uint8_t opcode = next_u8(&ptr);

    sprintf(debug_string, "%02X", opcode);

    switch (opcode) {
    case BP_SUBSCRIBE: {
        uint16_t uuid = next_u16_le(&ptr);
        if (uuid == FTMS_TREADMILL_DATA_UUID) {
            pTreadmill->hw.ble.ftms.subscribed = 1;
        }
        // movble_treadmill_subscribed(ptr);
        break;
    }
    case BP_UNSUBSCRIBE: {
        uint16_t uuid = next_u16_le(&ptr);
        if (uuid == FTMS_TREADMILL_DATA_UUID) {
            pTreadmill->hw.ble.ftms.subscribed = 0;
        }
        break;
    }
    case BP_CONTROL_POINT: {
        ble_ftms_control_point(ptr);
        break;
    }
    case BP_LINKINFO: {
        pTreadmill->hw.ble.connection.interval = next_u16_le(&ptr);
        pTreadmill->hw.ble.connection.latency = next_u16_le(&ptr);
        pTreadmill->hw.ble.connection.timeout = next_u16_le(&ptr);
        pTreadmill->hw.ble.connection.mtu = next_u16_le(&ptr);
        break;
    }
    case BP_BLE_HEARTRATE_CONNECTING: {
        pTreadmill->hw.ble.heartrate.connecting = 1;
        break;
    }
    case BP_BLE_HEARTRATE: {
        pTreadmill->hw.ble.heartrate.connecting = 0;
        pTreadmill->hw.ble.heartrate.bpm = next_u8(&ptr);
        break;
    }
    case BP_BLE_HEARTRATE_DISCONNECT: {
        pTreadmill->hw.ble.heartrate.connecting = 0;
//        pTreadmill->hw.ble.hrm.status = HRM_STATUS_DISCONNECTED;
//        pTreadmill->hw.ble.hrm.bpm = 0;
//        memset(pTreadmill->hw.ble.hrm.device_name, 0, sizeof(pTreadmill->hw.ble.hrm.device_name));
        break;
    }
    case BP_BLE_HRM: {
        uint8_t hrm_opcode = next_u8(&ptr);
        switch (hrm_opcode) {
            case BP_BLE_HRM_DEVICE_NAME: {
                uint8_t name_length = next_u8(&ptr);
                char hrm_device_name[32] = {0};
                for (int i = 0; i < name_length; ++i) {
                    hrm_device_name[i] = next_u8(&ptr);
                }
                // Armazena o nome do dispositivo HRM encontrado
//                strncpy(pTreadmill->hw.ble.hrm.device_name, hrm_device_name, name_length);
                break;
            }
            case BP_BLE_HRM_CONNECTING: {
//                pTreadmill->hw.ble.hrm.status = HRM_STATUS_CONNECTING;
                break;
            }
            case BP_BLE_HRM_CONNECTED: {
//                pTreadmill->hw.ble.hrm.status = HRM_STATUS_CONNECTED;
                break;
            }
            case BP_BLE_HRM_NOTIFICATION: {
               pTreadmill->hw.ble.heartrate.bpm = next_u8(&ptr);
                break;
            }
            case BP_BLE_HRM_DISCONNECTED: {
//                pTreadmill->hw.ble.hrm.status = HRM_STATUS_DISCONNECTED;
//                pTreadmill->hw.ble.hrm.bpm = 0;
//                memset(pTreadmill->hw.ble.hrm.device_name, 0, sizeof(pTreadmill->hw.ble.hrm.device_name));
                break;
            }
            default:
                break;
        }
        break;
    }
    case BP_GK_NOTIFICATION: {
        uint8_t length = next_u8(&ptr);
        uint8_t buffer[128] = {0};
        for (int i = 0; i < length; ++i) {
            buffer[i] = next_u8(&ptr);
        }
        // now we should process the buffer
        uint8_t i = 0;
        uint8_t reserved = buffer[i++];
        (void)reserved;
        while (i < length) {
            uint8_t type = buffer[i++];
            uint8_t len = buffer[i++];
            switch (type) {
            case 0x01: {
                uint8_t flags = buffer[i++];
                if (flags & 0x01) {
                    // heart rate disabled
                } else if (flags & 0x02) {
                    // temporaly unavailable
                } else {
                    // there's bpm data
                    // assert(len == 3 && "Heart Rate Len is not size 3");
                    if (len == 3){
                        pTreadmill->hw.ble.gk.bpm = buffer[i] | buffer[i + 1] << 8;
                        i += 2;
                    }
                }
                break;
            }
            case 0x02: {
                // active energy parameter
                uint8_t flags = buffer[i++];

                if (flags & 0x1) {
                    // temporarily unavailable
                } else {
                    // assert(len == 3 && "Active Energy Len is not size 3");
                    if (len == 3){
                        pTreadmill->hw.ble.gk.active_energy = buffer[i] | buffer[i + 1] << 8;
                        i += 2;
                    }
                    // queue_active_kcal_uart;
                }
                break;
            }
            case 0x03: {
                // total energy parameter
                uint8_t flags = buffer[i++];
                if (flags & 0x01) {
                    // temporaliry unavailable
                } else {
                    // assert(len == 3 && "Total Energy Len is not size 3");
                    // queue_kcal_uart
                    if (len == 3){
                        pTreadmill->hw.ble.gk.total_energy = buffer[i] | buffer[i + 1] << 8;
                        i += 2;
                    }
                }
                break;
            }
            case 0x04: {
                // average heart rate parameter
                uint8_t flags = buffer[i++];
                if (flags & 0x01) {
                    // disabled
                } else if (flags & 0x02) {
                    // temporarily unavailable
                } else {
                    //assert(len == 3 && "Average BPM is not size 3");
                    if (len == 3) {
                    	pTreadmill->hw.ble.gk.average_bpm = buffer[i] | buffer[i + 1] << 8;
						i += 2;
                    }

                    // queue_average_bpm_uart;
                }
                break;
            }
            }
        }
        pTreadmill->hw.ble.gk.last_time_received = pTreadmill->get_time_ms();
        break;
    }
    default:;
    }
}

static void ble_connected(ble_t *ble) { ble->connected = 1; }

static void ble_disconnected(ble_t *ble) {
    ble->connected = 0;
    ble->ftms.control_point_allowed = 0;
    ble->ftms.subscribed = 0;
    ble->connection.interval = 0;
    ble->connection.latency = 0;
    ble->connection.timeout = 0;
    ble->connection.mtu = 0;
}

static void ble_interpret_message_bytes(ble_t *ble, char *data, int length) {
    // TODO: refactor. It is this way in case recepetion is made up with smaller
    // messages
    static uint8_t mBuffer[256] = {0};
    static int mBufferIndex = 0;
    for (int i = 0; i < length; ++i) {
        uint8_t byte = (uint8_t)data[i];
        switch (byte) {
        case 0x01:
        case 0x02:
            mBufferIndex = 0;
            break;

        case 0x03:
        case 0x04:
        case 0x0A: {
            mBuffer[mBufferIndex] = 0;
            ble_legacy_commands_handler((char *)mBuffer); // ok
            break;
        }

        case 0xF0:
        case 0xF1:
            mBufferIndex = 0;
            mBuffer[mBufferIndex++] = byte;
            break;

        case 0xF5:
        case 0xF6:
            mBuffer[mBufferIndex] = '\0';
            ble_protocol_process(mBuffer, mBufferIndex); // ok
            break;

        case 0x0F:
            ble_connected(ble); // ok
            break;

        case 0x19:
        case 0x0E:
            ble_disconnected(ble); // ok
            break;

        default: {
            mBuffer[mBufferIndex] = byte;
            mBufferIndex = (mBufferIndex + 1) % 256;
        }
        }
    }
}

int ble_process_received(ble_t *ble, char *data, int length) {
    // ble_interpret_message_bytes(ble, data, length);
    //  If there's a command processing

    if (tail != head) {
        ble_command_t *cmd = &commands[tail];
        switch (cmd->status) {
        case BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE: {
            if (length <= 0)
                break;
            // temp_debug_print(data, length);
            if (is_expected_response(cmd, data, length)) {
                cmd->status = BLE_COMMAND_STATUS_OK;
                return 1;
            } else if (is_error_response(cmd, data, length)) {
                cmd->status = BLE_COMMAND_STATUS_ERROR;
                return 1;
            } else if (is_timedout(ble, cmd)) {
                cmd->status = BLE_COMMAND_STATUS_TIMEDOUT;
            }
            break;
        }
        default:;
        }

        if (length <= 0)
            return 1;
    }
    return 1;
}

int ble_process_commands(ble_t *ble) {
    if (head != tail) {
        ble_command_t *cmd = &commands[tail];
        switch (cmd->status) {
        case BLE_COMMAND_STATUS_PENDING: {
            ble->send_data((uint8_t *)cmd->data, strlen(cmd->data));
            if (strlen(cmd->response) > 0 && cmd->timeout_tick > 0) {
                cmd->status = BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE;
                cmd->tick = ble->get_tick();
            } else {
                cmd->status = BLE_COMMAND_STATUS_OK;
            }
            break;
        }
        case BLE_COMMAND_STATUS_WAIT_FOR_RESPONSE: {
            if (is_timedout(ble, cmd)) {
                cmd->status = BLE_COMMAND_STATUS_TIMEDOUT;
            }
            break;
        }
        case BLE_COMMAND_STATUS_OK: {
            tail = (tail + 1) % MAX_NUM_COMMANDS;
            break;
        }
        case BLE_COMMAND_STATUS_TIMEDOUT: {
            return BLE_PROCESS_STATUS_ERROR;
        }
        case BLE_COMMAND_STATUS_ERROR: {
            return BLE_PROCESS_STATUS_ERROR;
        }
        }

        return BLE_PROCESS_STATUS_OK;

    } else {
        return BLE_PROCESS_STATUS_IDLE;
    }
}

void ble_reset(ble_t *ble) {
    memset(ble, 0x00, sizeof(ble_t));
    ble->state = BLE_STATE_INIT;
}
