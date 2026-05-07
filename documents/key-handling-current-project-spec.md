# Current Project Key Handling Spec

## Scope

This document describes how keys are handled in the current project, from physical
input source to normalized key, router behavior, scene action, error detection,
persistence, and clearing/reset flows.

The spec is source-derived from the current branch. It focuses on firmware behavior
under `rt_led/` and `shared/`, with a short note about the desktop simulator path.
Hardware timing and simultaneous-key edge cases should still be validated on target
hardware.

## High-Level Pipeline

```text
Physical input
  -> source-specific scanner/decoder
  -> normalized kbd_key_t value
  -> keyboard router, for firmware keyboard sources
  -> treadmill_key_press()
  -> rt_led_key_press()
  -> current scene keypress()
  -> treadmill action/config/program/error side effects
```

Desktop simulator input calls `treadmill_key_press()` directly and therefore bypasses
the firmware keyboard router health checks.

## Key Vocabulary

The canonical key type is `kbd_key_t` in `shared/treadmill_key.h`.

| Key | Meaning |
| --- | --- |
| `KEY_NONE` | No key |
| `KEY_STOP` | Main stop |
| `KEY_SPEED_DECREMENT` | Main speed minus |
| `KEY_SPEED_6` | Main direct speed 6.0 km/h |
| `KEY_SPEED_9` | Main direct speed 9.0 km/h |
| `KEY_SPEED_12` | Main direct speed 12.0 km/h |
| `KEY_SPEED_15` | Main direct speed 15.0 km/h |
| `KEY_SPEED_INCREMENT` | Main speed plus |
| `KEY_START` | Main start |
| `KEY_PROGRAM` | Program key |
| `KEY_INCLINE_DECREMENT` | Main incline minus |
| `KEY_INCLINE_INCREMENT` | Main incline plus |
| `KEY_INCLINE_3` | Direct incline/program shortcut 3 |
| `KEY_INCLINE_6` | Direct incline/program shortcut 6 |
| `KEY_INCLINE_9` | Direct incline/program shortcut 9 |
| `KEY_INCLINE_12` | Direct incline/program shortcut 12 |
| `KEY_SAC` | SAC key |
| `KEY_CONFIG` | Config key |
| `KEY_LUB` | Lubrication combo key |
| `KEY_FAN` | Fan key, declared but not handled in current scene code |
| `KEY_EXT_KBD_SPEED_6` | External direct speed 6.0 km/h |
| `KEY_EXT_KBD_SPEED_9` | External direct speed 9.0 km/h |
| `KEY_EXT_KBD_SPEED_12` | External direct speed 12.0 km/h |
| `KEY_EXT_KBD_SPEED_15` | External direct speed 15.0 km/h |
| `KEY_EXT_KBD_INCLINE_INC` | External incline increment, declared but not routed by current I2C decoder |
| `KEY_EXT_KBD_INCLINE_DEC` | External incline decrement, declared but not routed by current I2C decoder |
| `KEY_EXT_KBD_SPEED_INC` | External/handgrip speed plus |
| `KEY_EXT_KBD_SPEED_DEC` | External/handgrip speed minus |
| `KEY_EXT_KBD_STOP` | External/handgrip stop |
| `KEY_EXT_KBD_START` | External/handgrip start |

## Input Sources

Firmware keyboard routing classifies inputs by source:

| Source enum | Input hardware | Source group | Fault lock bit |
| --- | --- | --- | --- |
| `KEYBOARD_INPUT_SOURCE_MAIN_MATRIX` | Main panel matrix scanned through GPIO rows/columns | Main | `KEYBOARD_SOURCE_MASK_MAIN` |
| `KEYBOARD_INPUT_SOURCE_MAIN_I2C` | External I2C keyboard controller at address `0x18` | Main | `KEYBOARD_SOURCE_MASK_MAIN` |
| `KEYBOARD_INPUT_SOURCE_HG_SPEED` | Handgrip speed plus/minus GPIOs | Handgrip speed | `KEYBOARD_SOURCE_MASK_HG_SPEED` |
| `KEYBOARD_INPUT_SOURCE_HG_START_STOP` | Handgrip start/stop GPIOs | Handgrip start/stop | `KEYBOARD_SOURCE_MASK_HG_START_STOP` |

Main matrix and main I2C share the same source group and lock bit. A main-matrix fault
and a main-I2C fault both disable the main keyboard group, except for the documented
safety/maintenance exceptions.

## Source Mapping

### Main Matrix

The main matrix scanner reads four samples and ANDs the results to stabilize the
pressed-key bitmask.

| Matrix function result | Normalized key |
| --- | --- |
| Stop matrix position | `KEY_STOP` |
| Speed minus matrix position | `KEY_SPEED_DECREMENT` |
| Speed 6 matrix position | `KEY_SPEED_6` |
| Speed 9 matrix position | `KEY_SPEED_9` |
| Speed 12 matrix position | `KEY_SPEED_12` |
| Speed 15 matrix position | `KEY_SPEED_15` |
| Speed plus matrix position | `KEY_SPEED_INCREMENT` |
| Start matrix position | `KEY_START` |
| Program matrix position | `KEY_PROGRAM` |
| Incline minus matrix position, incline models only | `KEY_INCLINE_DECREMENT` |
| Incline plus matrix position, incline models only | `KEY_INCLINE_INCREMENT` |
| Incline/program shortcut 3 | `KEY_INCLINE_3` |
| Incline/program shortcut 6 | `KEY_INCLINE_6` |
| Incline/program shortcut 9 | `KEY_INCLINE_9` |
| Incline/program shortcut 12 | `KEY_INCLINE_12` |
| SAC matrix position | `KEY_SAC` |

Matrix-derived combo keys:

| Physical combo | Normalized key | Timing/action layer |
| --- | --- | --- |
| START + STOP | `KEY_CONFIG` | Main matrix handler requires 2 seconds before dispatch |
| SPEED+ + SPEED- | `KEY_LUB` | Main matrix handler requires 2 seconds before dispatch |

The LUB entry action from idle is currently commented out, so the combo can be
generated but does not enter `SCENE_LUB` from idle in the current code.

### External I2C Keyboard

The I2C decoder reads one key code from the keyboard controller and routes the
normalized key as `KEYBOARD_INPUT_SOURCE_MAIN_I2C`.

| I2C code | Normalized key/action |
| --- | --- |
| `0x01` | `KEY_EXT_KBD_SPEED_9` |
| `0x02` | `KEY_EXT_KBD_SPEED_6` |
| `0x03` | `KEY_SPEED_DECREMENT` |
| `0x04` | `KEY_EXT_KBD_SPEED_12` |
| `0x05` | If no incline, `KEY_SPEED_DECREMENT`; otherwise `KEY_INCLINE_INCREMENT` |
| `0x06` | `KEY_SPEED_INCREMENT` |
| `0x07` | `KEY_EXT_KBD_SPEED_15` |
| `0x08` | `KEY_INCLINE_DECREMENT` |
| `0x09` | `KEY_STOP` |
| `0x0A` | `KEY_START` |
| `0x0B` | In idle, hold 1200 ms starts HRM scan prompt; otherwise `KEY_PROGRAM`. On release before scan, dispatches `KEY_PROGRAM` |
| `0x0C` | `KEY_INCLINE_6` |
| `0x0D` | `KEY_INCLINE_9` |
| `0x0E` | `KEY_INCLINE_12` |
| `0x0F` | Hold 1200 ms dispatches `KEY_CONFIG`. In CONFIG, allowed again only after scene age > 1000 ms |
| `0x10` | `KEY_PROGRAM` |
| `0xff` | Release event for `KEYBOARD_INPUT_SOURCE_MAIN_I2C` |

I2C-specific behaviors:

- The state machine polls pressed keys every 20 ms until `0xff` release.
- It supports repeat intervals for selected speed/incline keys in program contexts.
- It tracks speed plus/minus events within 150 ms to activate the idle keyboard
  clear/unlock combo.
- It suppresses repeated old direct-speed shortcut codes during one I2C press so the
  latest distinct shortcut is dispatched on release.

### Handgrip Speed and Start/Stop GPIOs

The TIM1 callback samples handgrip pins when `enable_shortcut_keys_reading == 1`.

| GPIO path | Normalized key | Source |
| --- | --- | --- |
| Speed plus path | `KEY_EXT_KBD_SPEED_INC` | `KEYBOARD_INPUT_SOURCE_HG_SPEED` |
| Speed minus path | `KEY_EXT_KBD_SPEED_DEC` | `KEYBOARD_INPUT_SOURCE_HG_SPEED` |
| Start path | `KEY_EXT_KBD_START` | `KEYBOARD_INPUT_SOURCE_HG_START_STOP` |
| Stop path | `KEY_EXT_KBD_STOP` | `KEYBOARD_INPUT_SOURCE_HG_START_STOP` |

Handgrip debounce/filter rules:

- A valid press requires high duration >= 50 ms and low duration >= 50 ms.
- `KEY_EXT_KBD_SPEED_INC` is throttled to one accepted press per 1000 ms.
- Held handgrip keys call `keyboard_route_key_hold()` so stuck faults can be detected
  without waiting for another edge.
- Release is source-based. When all pins in a handgrip source return to released, the
  source gets a `keyboard_route_key_release()`.

Handgrip start/stop combo:

- START + STOP held together suppresses individual START/STOP.
- After 2000 ms, it emits `KEY_CONFIG` from the handgrip start/stop source.
- It emits CONFIG once per hold.
- On release, the handgrip start/stop source is released and the shortcut state is
  cleared.

### Desktop Simulator Input

The desktop simulator is not routed through firmware keyboard health checks. It calls
`treadmill_key_press()` or action helpers directly.

| Desktop character | Action |
| --- | --- |
| `s` | `KEY_START` |
| `p` | `KEY_STOP` |
| `q` | `KEY_PROGRAM` |
| `c` | `KEY_CONFIG` |
| `=` | `KEY_SPEED_INCREMENT` |
| `-` | `KEY_SPEED_DECREMENT` |
| `0` | `KEY_INCLINE_INCREMENT` |
| `9` | `KEY_INCLINE_DECREMENT` |
| `o` | `KEY_PROGRAM` |
| `e` | `treadmill_emergency_pressed()` |
| `n` | Intended BPM stop path in desktop code; current call is stale relative to the shared function signature |
| `m` | `treadmill_set_bpm_read()` |
| `,` | Calls BPM minus 20 and then falls through into the `.` case in current code |
| `.` | BPM plus 20 |
| `l` | `KEY_LUB` |

Because the desktop simulator bypasses `keyboard_route_key_event()`, keyboard stuck,
burst, source lockout, release-based shortcut dispatch, and C2V confirmation are not
faithfully simulated by those desktop key presses.

## Firmware Router Behavior

The router records per-source health:

| Field | Meaning |
| --- | --- |
| `pressed` | Whether that source currently has an active routed key |
| `last_key` | Last routed key for that source |
| `press_start_ms` | Time when the active press started |
| `burst_window_start_ms` | Start time for burst/noise detection |
| `burst_count` | Number of new presses in the current burst window |

Router entry points:

| Function | Caller | Purpose |
| --- | --- | --- |
| `keyboard_route_key_event(source, key)` | Matrix, I2C, handgrip scanner | Normal key press/repeat event |
| `keyboard_route_key_hold(source, key)` | Handgrip scanner | Held-key monitoring without a new edge |
| `keyboard_route_key_release(source)` | Matrix no-key, I2C `0xff`, handgrip release | Release-based dispatch and cleanup |

Router decisions, in order:

1. Update/expire C2V pending shortcut state.
2. Suppress health faults if the idle clear/unlock combo is active for the current
   plus/minus key.
3. Apply I2C direct-speed shortcut re-entry filtering, for main I2C only.
4. If the source group is fault-disabled, allow only safety/maintenance exceptions.
5. Update source health and burst counter.
6. Detect burst fault.
7. Detect stuck fault.
8. Delay release-action keys.
9. Apply normal key action through `treadmill_key_press()`.

## Release-Action Keys

Some keys are intentionally not dispatched on the initial press. They are dispatched
from `keyboard_route_key_release()` after the key is released.

| Source | Keys dispatched on release |
| --- | --- |
| Main matrix | `KEY_SPEED_6`, `KEY_SPEED_9`, `KEY_SPEED_12`, `KEY_SPEED_15`, `KEY_START` |
| Main I2C | `KEY_EXT_KBD_SPEED_6`, `KEY_EXT_KBD_SPEED_9`, `KEY_EXT_KBD_SPEED_12`, `KEY_EXT_KBD_SPEED_15`, `KEY_START` |
| Handgrip start/stop | `KEY_EXT_KBD_START` |

Exception:

- In `SCENE_TRAINING`, when C2V is disabled, a direct speed shortcut whose target is
  below the current speed dispatches on press rather than release.
- Repeated held events for that same shortcut are ignored after the first dispatch.

Long-press ignore:

- In `SCENE_TRAINING`, when C2V is disabled, a direct speed shortcut held for
  >= 3000 ms is ignored on release.
- If it reaches the 8000 ms stuck threshold, the stuck fault path applies instead.

## C2V Confirmation Path

C2V is the persisted config option `treadmill->config.c2v`, stored at `NVM_U8_C2V`.

When C2V is enabled:

| User action in training | Router behavior |
| --- | --- |
| Direct speed shortcut target > current speed | Store pending shortcut, show `START`, blink pending speed for 6000 ms |
| START release within 6000 ms | Apply the stored shortcut key |
| Window expires | Clear pending shortcut with no speed change |
| Scene leaves training | Clear pending shortcut |
| C2V disabled while pending | Clear pending shortcut |
| Direct speed shortcut target <= current speed | Dispatch normally, no confirmation |

Display feedback:

- Dot matrix message: `START`.
- Seven-segment speed field: pending speed, blinking every 500 ms.

## Dispatch Into Treadmill and Scene Layer

Once the router accepts a key, it calls `treadmill_key_press()`.

`treadmill_key_press()`:

1. If the key changed, updates:
   - `treadmill->key_pressed`
   - `treadmill->key_pressed_tick`
   - `treadmill->key_pressed_last_tick`
2. Calls `rt_led_key_press()`.

`rt_led_key_press()`:

1. Looks up the current scene in the registered scene table.
2. Emits `BEEP_KEYPRESS_ACK`, except:
   - Idle START / external START does not beep because countdown will beep.
   - LUB scene suppresses repeated keypress beep while the key is held.
3. Calls the current scene's `keypress()` handler.

If the scene is not registered, the key is ignored. `SCENE_RESET` exists in the enum
but has no registered scene handler in `rt_led_scenes`.

## Scene Action Matrix

This matrix describes what a key does after it reaches the scene layer. Keys not
listed for a scene are ignored by that scene, unless the router blocks them earlier.

| Scene | Key | Action |
| --- | --- | --- |
| `SCENE_RESET` | Any | No registered scene handler; ignored |
| `SCENE_INIT` | Any | Ignored |
| `SCENE_INIT` update | Time >= 1500 ms | Set scene to idle |
| `SCENE_IDLE` | START / external START | Start countdown |
| `SCENE_IDLE` | PROGRAM | If modbus config is enabled, select next modbus parameter; otherwise enter program selection at `PROGRAM_TIME` |
| `SCENE_IDLE` | INCLINE_6 | Select `PROGRAM_WALK_30MIN`, load defaults, enter program scene |
| `SCENE_IDLE` | INCLINE_9 | Select `PROGRAM_200K_CALORIES`, load defaults, enter program scene |
| `SCENE_IDLE` | INCLINE_12 | Select `PROGRAM_WALK_08MIN`, load defaults, enter program scene |
| `SCENE_IDLE` | CONFIG | Enter config scene at option 0 |
| `SCENE_IDLE` update | None | No key-driven auto transition in current enabled code |
| `SCENE_CONFIG` | START / external START | Advance to next config option |
| `SCENE_CONFIG` | STOP / external STOP | Reset ERR index, save config, return idle |
| `SCENE_CONFIG` | SPEED+ / SPEED- / external SPEED+ / external SPEED- | Change current config option, see config matrix |
| `SCENE_CONFIG` | CONFIG while option is ERR | Reset all errors, show reset message, return idle |
| `SCENE_PROGRAM` | STOP / external STOP | Return idle; program handler also exits program |
| `SCENE_PROGRAM` | Other keys | Delegated by current `program.id`, see program matrix |
| `SCENE_PROGRAM` update | 30 seconds no interaction | Return idle |
| `SCENE_COUNTDOWN` | STOP / external STOP | Reset dot-matrix frame, abort countdown, exit program, return idle |
| `SCENE_COUNTDOWN` update | Countdown reaches zero | Beep countdown end, start training |
| `SCENE_TRAINING` | STOP / external STOP | Pause training; if test mode enabled, disable test mode |
| `SCENE_TRAINING` | SPEED+ | Increment speed by `config.speed.step` |
| `SCENE_TRAINING` | external SPEED+ | Increment speed by 50, clamped to `config.speed.max` |
| `SCENE_TRAINING` | SPEED- | Decrement speed by `config.speed.step` |
| `SCENE_TRAINING` | external SPEED- | Decrement speed by 100, clamped to `config.speed.min` |
| `SCENE_TRAINING` | INCLINE+ | Increment incline by `config.incline.step` |
| `SCENE_TRAINING` | INCLINE- | Decrement incline by `config.incline.step` |
| `SCENE_TRAINING` | SPEED_6 / external SPEED_6 | Set speed to 600 |
| `SCENE_TRAINING` | SPEED_9 / external SPEED_9 | Set speed to 900 |
| `SCENE_TRAINING` | SPEED_12 / external SPEED_12 | Set speed to 1200 |
| `SCENE_TRAINING` | SPEED_15 / external SPEED_15 | Set speed to 1500 |
| `SCENE_TRAINING` | PROGRAM | If modbus config is enabled, select next modbus parameter |
| `SCENE_TRAINING` | INCLINE_3/6/9/12 | No action |
| `SCENE_TRAINING` update | Test mode enabled | Run test-program speed/incline cycle once per second |
| `SCENE_PAUSED` | START / external START | Resume training |
| `SCENE_PAUSED` | STOP / external STOP | Reset dot-matrix frame, finish training, enter report |
| `SCENE_PAUSED` | SPEED+/SPEED-/INCLINE+/INCLINE- | Explicit no-op |
| `SCENE_PAUSED` update | 2 minutes elapsed | Finish training, enter report |
| `SCENE_REPORT` | START / external START | Enable NFC, close report, save totals, reset training, return idle |
| `SCENE_REPORT` | STOP / external STOP | Same as START in report |
| `SCENE_REPORT` update | 30 seconds idle | Close report and return idle |
| `SCENE_EMERGENCY` | Any key | Only refreshes idle tick |
| `SCENE_EMERGENCY` update | 5 seconds elapsed and EMG pin released | Return idle |
| `SCENE_LUB` | Any key except `KEY_NONE` and `KEY_LUB` | Return idle |
| `SCENE_LUB` update | `KEY_LUB` held 3000 ms | Reset lubrication state |
| `SCENE_LUB` update | Timeout after release | Return idle |
| `SCENE_BLE_ALLOW_YES_NO` | START / external START | Build/send HRM connect packet, mark BLE permission allowed, return idle |
| `SCENE_BLE_ALLOW_YES_NO` | STOP / external STOP | Build/send HRM disconnect packet, deny permission, clear HRM state, return idle |
| `SCENE_BLE_ALLOW_YES_NO` update | 15 seconds without permission allowed | Return idle |
| `SCENE_TEST` | START / external START | Start countdown |
| `SCENE_TEST` | STOP / external STOP | Disable test mode, return to previous scene |
| `SCENE_TEST` | PROGRAM / CONFIG / LUB / SAC | Explicit no-op |

## Config Option Matrix

Config scene uses START to move through options and speed plus/minus to edit the
current option.

| Config option | Display intent | SPEED+ behavior | SPEED- behavior | Extra keys/notes |
| --- | --- | --- | --- | --- |
| `CONFIG_OPTION_MODEL` | Model | Increment model, wrap, reload model params, restart BLE | Decrement model, wrap, reload model params, restart BLE |  |
| `CONFIG_OPTION_LANGUAGE` | Language | Increment language, wrap, restart BLE | Decrement language, wrap, restart BLE |  |
| `CONFIG_OPTION_UNIT` | Unit | Increment/wrap unit | Decrement/wrap unit |  |
| `CONFIG_OPTION_WEIGHT` | Weight | Increment weight, wrap to min | Decrement weight, wrap to max |  |
| `CONFIG_OPTION_MA` | Maintenance/MA | No key edit implemented | No key edit implemented |  |
| `CONFIG_OPTION_NAME` | BLE/device name id | Increment `device_name_id`, wrap to 1 | Decrement down to 1, otherwise set 1 | Saved on config exit |
| `CONFIG_OPTION_VER` | Version | No key edit implemented | No key edit implemented |  |
| `CONFIG_OPTION_TOTAL_DISTANCE` | Total distance | No key edit implemented | No key edit implemented |  |
| `CONFIG_OPTION_TOTAL_HOURS` | Total hours | No key edit implemented | No key edit implemented |  |
| `CONFIG_OPTION_BLE_STATE` | BLE state/DFU advertising | No key edit implemented | No key edit implemented | Entering option sends `dfu adv=1`; exiting sends `dfu adv=0` |
| `CONFIG_OPTION_MODBUS` | Modbus parameter display | Toggle show/hide | Toggle show/hide |  |
| `CONFIG_OPTION_TEST` | Test mode | Toggle; if enabled, enter `SCENE_TEST` | Toggle; if enabled, enter `SCENE_TEST` |  |
| `CONFIG_OPTION_C2V` | C2V/C2F shortcut confirmation | Toggle enable/disable | Toggle enable/disable | Stored at config exit |
| `CONFIG_OPTION_ERRORS` | Error browser | Advance error index and wrap | No edit implemented in `change_config_errors()` | `KEY_CONFIG` resets all errors and returns idle |
| `CONFIG_OPTION_MAX_TIME` | Max quick-mode time | If zero, set lower bound; otherwise add 60 seconds up to default max, then wrap to zero | Subtract 60 seconds down to lower bound, otherwise zero | Keypress dispatch only calls this helper for speed plus/minus keys |

On STOP / external STOP, `treadmill_exit_config()` saves changed config values and
returns to idle.

## Program Selection Matrix

`SCENE_PROGRAM` delegates key handling based on `treadmill->program.id`.

Common program keys:

| Program | START | STOP | PROGRAM |
| --- | --- | --- | --- |
| Time | Start countdown | Exit to idle | Next program |
| Distance | Start countdown | Exit to idle | Next program |
| Calories | Start countdown | Exit to idle | Next program |
| 200 kcal | Start countdown | Exit to idle | Next program |
| HIIT | Advance parameter until final parameter, then start countdown | Exit to idle | Next program |
| Walk 30 min | Start countdown | Exit to idle | Next program |
| Walk 08 min | Start countdown | Exit to idle | Next program |
| Fat burn | Advance from time to age, then compute BPM zone and start countdown | Exit to idle | Next program |
| Super cardio | Advance from time to age, then compute BPM zone and start countdown | Exit to idle | Next program |

Program parameter edits:

| Program | SPEED+ | SPEED- | Direct speed shortcuts | Incline shortcut keys |
| --- | --- | --- | --- | --- |
| Time | Add 60 seconds up to max | Subtract 60 seconds above min | Ignored | INCLINE_6 -> Walk 30, INCLINE_9 -> 200 kcal, INCLINE_12 -> Walk 08 |
| Distance | Add 100 m up to max | Subtract 100 m above min | Ignored | INCLINE_6 -> Walk 30, INCLINE_9 -> 200 kcal, INCLINE_12 -> Walk 08 |
| Calories | Add 5 kcal up to max | Subtract 5 kcal down to min | Ignored | INCLINE_6 -> Walk 30, INCLINE_9 -> 200 kcal, INCLINE_12 -> Walk 08 |
| 200 kcal | Add 200 kcal up to max | Subtract 200 kcal down to min | Ignored | INCLINE_6 -> Walk 30, INCLINE_9 no-op, INCLINE_12 -> Walk 08 |
| HIIT param 0 | Add 30 seconds to time 1 | Subtract 30 seconds from time 1 | Explicit no-op | INCLINE_6 -> Walk 30, INCLINE_9 -> 200 kcal, INCLINE_12 -> Walk 08 |
| HIIT param 1 | Add speed step to speed 1 | Subtract speed step from speed 1 | Explicit no-op | Same as HIIT param 0 |
| HIIT param 2 | Add 30 seconds to time 2 | Subtract 30 seconds from time 2 | Explicit no-op | Same as HIIT param 0 |
| HIIT param 3 | Add speed step to speed 2 | Subtract speed step from speed 2 | Explicit no-op | Same as HIIT param 0 |
| HIIT param 4 | Add one series up to max | Subtract one series down to min | Explicit no-op | Same as HIIT param 0 |
| Walk 30 min | Add speed step to target speed up to 8.0 km/h | Subtract speed step down to 3.0 km/h | Ignored; SPEED_6/SPEED_9 are explicit no-ops | INCLINE_6 no-op, INCLINE_9 -> 200 kcal, INCLINE_12 -> Walk 08 |
| Walk 08 min | Add speed step to target speed up to 15.0 km/h | Subtract speed step down to 1.2 km/h | Ignored; SPEED_6/SPEED_9 are explicit no-ops | INCLINE_6 -> Walk 30, INCLINE_9 -> 200 kcal, INCLINE_12 no-op |
| Fat burn param 0 | Add 60 seconds to duration | Subtract 60 seconds down to min | Ignored | INCLINE_6 -> Walk 30, INCLINE_9 -> 200 kcal, INCLINE_12 -> Walk 08 |
| Fat burn param 1 | Add one year to age | Subtract one year down to min | Ignored | Same as fat burn param 0 |
| Super cardio param 0 | Add 60 seconds to duration | Subtract 60 seconds down to min | Ignored | INCLINE_6 -> Walk 30, INCLINE_9 -> 200 kcal, INCLINE_12 -> Walk 08 |
| Super cardio param 1 | Add one year to age | Subtract one year down to min | Ignored | Same as super cardio param 0 |

External speed plus/minus keys are accepted by these handlers as aliases for the main
speed plus/minus keys in program selection.

## Training Action Details

When `SCENE_TRAINING` accepts a speed or incline key:

| Action | Function | Bounds |
| --- | --- | --- |
| Main speed plus | `treadmill_speed_increment()` | Requires current speed >= min and new speed <= max |
| Main speed minus | `treadmill_speed_decrement()` | Requires current speed >= min and new speed >= min |
| External speed plus | Direct calculation then `treadmill_set_speed()` | Adds 50 and clamps to max before set |
| External speed minus | Direct calculation then `treadmill_set_speed()` | Subtracts 100 and clamps to min before set |
| Direct speed | `treadmill_set_speed()` | Rejects speed > max or nonzero speed < min |
| Incline plus | `treadmill_incline_increment()` | Requires new incline <= max |
| Incline minus | `treadmill_incline_decrement()` | Requires new incline >= min |

`treadmill_set_speed()` also:

- Updates `training.speed`.
- Recomputes `training.pace`.
- Updates `aux.speed_target`.
- If in HIIT after training has started, updates the active HIIT speed slot.

`treadmill_set_incline()` also:

- Rejects values outside configured min/max.
- Updates `aux.incline_changed_time_ms` when incline changes.
- Updates `training.incline`.

## Scene Transition Actions

| Function | Main side effects |
| --- | --- |
| `treadmill_start_training_countdown()` | Reset countdown value/timer, set scene to COUNTDOWN |
| `treadmill_start_training()` | Clear training struct, initialize speed to min, disable GymKit pairing, set scene TRAINING, send FTMS started/resumed callback |
| `treadmill_abort_countdown()` | Exit current program and return idle |
| `treadmill_pause_training()` | Clear DM message, set scene PAUSED, send paused callback, set speed 0 |
| `treadmill_resume_training()` | Return scene TRAINING, reset speed to min, send started/resumed callback |
| `treadmill_finish_training()` | Clear DM message, speed 0, incline 0, set scene REPORT, disconnect HRM, enable GymKit pairing, send stopped callback |
| `treadmill_close_report()` | Save total distance/time, reset training/program state, return idle, clear BLE permission |
| `treadmill_program()` | Select `PROGRAM_TIME`, load defaults, enter PROGRAM scene |
| `treadmill_next_program()` | Advance program id, reset program param index, load defaults |
| `treadmill_exit_program()` | Clear program state, return idle, reset DM frame messages |
| `treadmill_config()` | Start config at option 0, disable test flag, snapshot previous config, enter CONFIG |
| `treadmill_exit_config()` | Save changed config and return idle |

## Keyboard Fault Detection

Keyboard faults are handled in the firmware router before a key reaches
`treadmill_key_press()`.

| Fault type | Trigger | Result |
| --- | --- | --- |
| Stuck key | Monitored key held >= 8000 ms | Source group lock bit is set and stuck fault is stored |
| Burst/noise | More than 8 new presses in a 1000 ms source window | Source group lock bit is set and burst fault is stored |

When stuck detection is active:

| Scene/context | Keys checked |
| --- | --- |
| Any scene | START / external START |
| Idle | Direct speed shortcut keys and handgrip speed plus/minus |
| Training with C2V disabled | Any non-`KEY_NONE` key |
| Training with C2V enabled | START keys only, because the all-key training rule is disabled by C2V |

Keyboard fault codes:

| Code | Symbol | Source group |
| --- | --- | --- |
| `0xA1` | `KEYBOARD_ERROR_MAIN_STUCK` | Main |
| `0xA2` | `KEYBOARD_ERROR_HG_START_STOP_STUCK` | Handgrip start/stop |
| `0xA3` | `KEYBOARD_ERROR_HG_SPEED_STUCK` | Handgrip speed |
| `0xA4` | `KEYBOARD_ERROR_MAIN_BURST` | Main |
| `0xA5` | `KEYBOARD_ERROR_HG_START_STOP_BURST` | Handgrip start/stop |
| `0xA6` | `KEYBOARD_ERROR_HG_SPEED_BURST` | Handgrip speed |

When a source group is locked:

| Incoming key | Behavior |
| --- | --- |
| STOP / external STOP | Allowed |
| CONFIG | Allowed |
| START / external START in CONFIG, when not currently stuck | Allowed for config navigation |
| `KEY_SPEED_INCREMENT` in CONFIG/ERR | Allowed for browsing errors |
| Any other key from locked group | Ignored |

Keyboard error registration returns early in CONFIG scene. A router-detected fault can
still set a runtime source lock while in CONFIG, but
`treadmill_register_keyboard_error()` does not persist or count keyboard errors from
that scene.

## Error Persistence

The project stores both inverter and keyboard errors in NVM.

Stored inverter data:

- 9 inverter error counters.
- Last 3 inverter error codes.
- Counts include overload at index 5.

Stored keyboard data:

- 6 keyboard error counters.
- Last 3 keyboard error codes.

Save behavior:

- Keyboard and inverter registration functions save immediately when the 5000 ms save
  debounce permits it.
- Otherwise they mark the corresponding error block dirty.
- `treadmill_update()` flushes dirty inverter/keyboard errors once the debounce window
  has elapsed.

Load behavior:

- `treadmill_load_config()` calls `treadmill_load_errors()`.
- Error counts and last-error histories are loaded from NVM.
- Runtime timestamps are reset to zero.
- `disabled_sources_mask` is rebuilt from keyboard last-error history, so keyboard
  lockout can persist across reboot.

Latest error selection:

- `treadmill_get_latest_error_code()` compares runtime timestamps across inverter and
  keyboard errors.
- If no timestamp exists, it prefers keyboard last error 0, then inverter last error 0.

## Error Display and Browsing

### Idle Display

In idle, seven-segment program display shows keyboard errors only:

| Latest error | Idle behavior |
| --- | --- |
| `0xA1..0xA6` | Program field shows ` E` plus two hex characters, such as ` EA1` |
| Other inverter/non-keyboard error | Program field hidden |
| No error | Program field hidden |

### CONFIG/ERR Browser

`CONFIG_OPTION_ERRORS` browses 18 entries:

```text
9 inverter counters + 3 inverter last-error slots + 6 keyboard counters
```

| ERR index range | Dot matrix | Seven-segment pace field |
| --- | --- | --- |
| `0..8` | `E*XX`, where `XX` is inverter error code | Counter for that inverter error |
| `9..11` | `E*XX`, where `XX` is stored inverter last-error code | `L01`, `L02`, or `L03` |
| `12..17` | `E*XX`, where `XX` is keyboard error code | Counter for that keyboard error |

ERR navigation:

- `KEY_SPEED_INCREMENT` advances the index and wraps to zero.
- `KEY_CONFIG` while on ERR resets all inverter and keyboard errors, shows reset
  message for 5000 ms, and returns idle.
- STOP / external STOP exits config and saves config.

## Error Clearing and Reset Matrix

| Flow | Where available | User action | What is cleared | What is not cleared |
| --- | --- | --- | --- | --- |
| Keyboard clear/unlock combo | Idle only | Hold speed plus and speed minus for 5000 ms on main matrix, handgrip speed, or I2C plus/minus within 150 ms | Keyboard last-error history in RAM/NVM, keyboard error timestamps, keyboard previous error, keyboard source lock bits, keyboard dirty state | Keyboard error counters, inverter counters/history |
| Full error reset | CONFIG scene, ERR option | Press CONFIG | All inverter counters, inverter last errors, keyboard counters, keyboard last errors, timestamps, dirty flags, keyboard source lock bits | Normal config values, total distance/time |
| Startup lock restoration | Boot/config load | Automatic | Restores keyboard source lock bits from keyboard last errors | Does not recreate timestamps |

The idle clear/unlock combo is meant to recover keyboard control without losing
service counters. CONFIG/ERR reset is the service reset for all stored error history
and counters.

## Inverter Error Codes

| Code | Symbol | Meaning |
| --- | --- | --- |
| `0x37` | `INVERTER_ERROR_OVERVOLTAGE` | Overvoltage |
| `0x34` | `INVERTER_ERROR_UNDERVOLTAGE` | Undervoltage |
| `0x43` | `INVERTER_ERROR_DC_UNDERVOLTAGE` | DC bus undervoltage |
| `0x3A` | `INVERTER_ERROR_OVERTEMPERATURE` | Overtemperature |
| `0x3D` | `INVERTER_ERROR_OVERCURRENT` | Overcurrent |
| `0x40` | `INVERTER_ERROR_OVERLOAD` | Overload |
| `0x55` | `INVERTER_ERROR_COMMUNICATION` | Communication failure |
| `0x52` | `INVERTER_ERROR_INCLINE_MOTOR` | Incline motor failure |
| `0x49` | `INVERTER_ERROR_MPHV` | MPHV |

Inverter errors are registered by the motor layer. A repeated identical inverter error
is suppressed when it occurs within 10 seconds of `last_ack_time`.

## End-to-End Paths

### Main Matrix START From Idle

1. Matrix scanner sees `KEY_START`.
2. Router records main-source press.
3. START is a release-action key, so press is not dispatched yet.
4. On release, router dispatches `KEY_START`.
5. `treadmill_key_press()` updates key timestamps.
6. `rt_led_key_press()` suppresses the normal key beep because idle START will start
   countdown beeps.
7. `scene_idle.keypress()` calls `treadmill_start_training_countdown()`.
8. Scene becomes `SCENE_COUNTDOWN`.

### Direct Speed Shortcut Increase With C2V Enabled

1. User releases direct shortcut, for example speed 12.
2. Router sees target 1200 is greater than current training speed.
3. Router stores pending shortcut and starts a 6000 ms window.
4. Dot matrix shows `START`; seven-segment speed blinks target speed.
5. User releases START within the window.
6. Router confirms pending shortcut and dispatches the stored speed key.
7. `scene_training.keypress()` sets speed to 1200.

### Stuck Key Fault

1. A monitored key remains held for 8000 ms.
2. Router sets the matching source-group disable mask.
3. Router calls `treadmill_register_keyboard_error()`.
4. Last keyboard errors shift, the relevant counter increments, and the error is
   saved or marked dirty.
5. Later keys from that source group are ignored except STOP, CONFIG, and service
   exceptions.
6. On reboot, `treadmill_load_errors()` rebuilds the lock bit from persisted keyboard
   last-error history.

### Idle Keyboard Unlock Without Counter Reset

1. In idle, user holds speed plus and speed minus.
2. Router suppresses faults for the combo keys.
3. After 5000 ms, `treadmill_clear_keyboard_error_code_and_unlock()` clears keyboard
   last errors and unlocks all keyboard source groups.
4. Keyboard counters remain available in CONFIG/ERR.

### CONFIG/ERR Full Reset

1. User enters config.
2. User navigates to ERR option.
3. User presses CONFIG.
4. `scene_config.keypress()` calls `treadmill_reset_errors()`.
5. Inverter and keyboard counters/history are zeroed in NVM and RAM.
6. Keyboard source lock bits are cleared.
7. Reset message is displayed for 5000 ms and scene returns idle.

## Current Behavior Caveats

- `SCENE_RESET` exists in the enum but is not registered in `rt_led_scenes`; if used,
  keypress/update/render routing finds no scene.
- Desktop simulator key input bypasses router-only behavior such as source lockout,
  stuck/burst detection, release-based dispatch, and C2V confirmation.
- `KEY_EXT_KBD_INCLINE_INC` and `KEY_EXT_KBD_INCLINE_DEC` are declared but the current
  I2C decoder routes I2C incline keys to the main incline keys instead.
- `KEY_FAN` is declared but no current scene handles it.
- Idle `KEY_LUB` scene entry is commented out, though `SCENE_LUB` still exists.
- CONFIG option `C2V` is displayed as `C2F` on the dot matrix in current code.
- In `SCENE_PROGRAM`, STOP is handled both by the scene wrapper and by program
  handlers. Both paths return to idle/exit program; this is redundant but consistent
  with the current implementation.
