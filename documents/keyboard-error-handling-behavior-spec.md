# Keyboard and Error Handling Behavior Spec

## Scope

This document describes the keyboard behavior and error handling implemented on branch
`Aria-1.2.4---ext-kbd-board-and-errors`, compared with
`origin/Aria-1.1.3-shortcut-kbd`.

The comparison range used for this document is:

```text
origin/Aria-1.1.3-shortcut-kbd...HEAD
```

The behavior below is derived from code inspection of the current branch. It was not
validated on hardware as part of writing this document.

## Summary of Recent Changes

The previous branch sent most keyboard detections directly to `treadmill_key_press()`.
The current branch routes all keyboard sources through a central keyboard router in
`rt_led/Core/Src/main.c`.

The router now owns:

- Source-aware press, hold, and release tracking.
- Stuck-key detection.
- Burst/noise detection.
- Source disable latches after keyboard faults.
- Safe exceptions for STOP, CONFIG, and selected config maintenance navigation.
- Release-based dispatch for START and speed shortcut keys.
- C2V confirmation behavior for speed shortcut increases.
- Keyboard fault clear/unlock by holding speed plus and speed minus in idle.

Error handling was expanded from inverter-only stored error counts/history to combined
inverter plus keyboard fault storage. The config ERR screen now browses inverter
counts, inverter last errors, and keyboard counts. The idle seven-segment program
field now shows keyboard error codes when a keyboard fault is active or restored from
NVM.

## Input Sources

The current branch classifies keyboard input into four sources:

| Source enum | Physical source | Source group | Disable mask |
| --- | --- | --- | --- |
| `KEYBOARD_INPUT_SOURCE_MAIN_MATRIX` | Main panel matrix scanned by `get_key_pressed()` | Main | `KEYBOARD_SOURCE_MASK_MAIN` |
| `KEYBOARD_INPUT_SOURCE_MAIN_I2C` | External I2C keyboard at address `0x18` | Main | `KEYBOARD_SOURCE_MASK_MAIN` |
| `KEYBOARD_INPUT_SOURCE_HG_SPEED` | Handgrip speed plus/minus GPIOs | HG speed | `KEYBOARD_SOURCE_MASK_HG_SPEED` |
| `KEYBOARD_INPUT_SOURCE_HG_START_STOP` | Handgrip start/stop GPIOs | HG start/stop | `KEYBOARD_SOURCE_MASK_HG_START_STOP` |

Main matrix and main I2C intentionally share one fault group. If either source causes
a main-keyboard fault, both main sources are locked by the same main disable bit.

## Event Routing Model

All non-emergency keyboard inputs are normalized through:

- `keyboard_route_key_event(source, key)`
- `keyboard_route_key_hold(source, key)`
- `keyboard_route_key_release(source)`

Each source stores:

- Whether a key is currently pressed.
- The last key for that source.
- Press start time.
- Burst detection window start.
- Burst count inside the active window.

This replaces the previous direct calls from:

- Main matrix handling.
- External I2C keyboard handling.
- Handgrip shortcut GPIO handling.

Emergency handling remains separate and still calls `treadmill_emergency_pressed()`
after its debounce logic.

## Main Matrix Behavior

The main matrix is still scanned through `get_key_pressed()`.

Existing matrix combination behavior remains:

- START plus STOP is converted to `KEY_CONFIG`.
- SPEED plus and SPEED minus is converted to `KEY_LUB`.

The current branch routes matrix keys through the keyboard router instead of direct
dispatch. The matrix handler still uses:

- 50 ms debounce before accepting a combined or single key.
- 2 seconds hold before dispatching combined keys such as CONFIG or LUB.
- Existing repeated-key pacing for held normal keys.

When the matrix reports no key pressed, the current branch calls
`keyboard_route_key_release(KEYBOARD_INPUT_SOURCE_MAIN_MATRIX)` and clears
`treadmill->key_pressed`.

The matrix plus/minus combo is also used as the main-keyboard clear/unlock combo in
idle mode.

## External I2C Keyboard Behavior

The external I2C keyboard state machine is unchanged in shape:

- `EXT_KBD_STATE_INIT` resets the I2C keyboard and initializes I2C2.
- `EXT_KBD_STATE_IDLE` polls version command `V` every 200 ms.
- IRQ moves idle to read.
- `EXT_KBD_STATE_READ` reads one key with command `R`.
- `EXT_KBD_STATE_PRESSED` polls the pressed key every 20 ms until release `0xff`.

Key mapping now routes through `keyboard_route_key_event()`:

| I2C code | Routed key |
| --- | --- |
| `0x01` | `KEY_EXT_KBD_SPEED_9` |
| `0x02` | `KEY_EXT_KBD_SPEED_6` |
| `0x03` | `KEY_SPEED_DECREMENT` |
| `0x04` | `KEY_EXT_KBD_SPEED_12` |
| `0x05` | `KEY_SPEED_DECREMENT` if no incline, otherwise `KEY_INCLINE_INCREMENT` |
| `0x06` | `KEY_SPEED_INCREMENT` |
| `0x07` | `KEY_EXT_KBD_SPEED_15` |
| `0x08` | `KEY_INCLINE_DECREMENT` |
| `0x09` | `KEY_STOP` |
| `0x0A` | `KEY_START` |
| `0x0B` | HRM BLE scan shortcut in idle after 1200 ms, otherwise `KEY_PROGRAM` |
| `0x0C` | `KEY_INCLINE_6` |
| `0x0D` | `KEY_INCLINE_9` |
| `0x0E` | `KEY_INCLINE_12` |
| `0x0F` | `KEY_CONFIG` after 1200 ms; allowed again in CONFIG after scene age > 1000 ms |
| `0x10` | `KEY_PROGRAM` |
| `0xff` | Release |

On release `0xff`, the current branch:

- Sends a pending `KEY_PROGRAM` if key `0x0B` was being used as a short press.
- Clears I2C config and BLE debounce state.
- Resets I2C clear-combo tracking.
- Calls `keyboard_route_key_release(KEYBOARD_INPUT_SOURCE_MAIN_I2C)`.

The current branch also tracks I2C speed plus/minus near-simultaneous presses. If
`KEY_SPEED_INCREMENT` and `KEY_SPEED_DECREMENT` are seen within 150 ms, the I2C source
marks the keyboard clear/unlock combo as active.

I2C shortcut re-entry protection was added for speed shortcut keys 6, 9, 12, and 15.
The I2C keyboard reports one key code at a time, and simultaneous shortcut presses can
transiently repeat an older key before release. The router records which shortcut
codes have already appeared during the current I2C press and ignores a repeated older
shortcut so the last distinct shortcut remains the one dispatched on release.

## Handgrip Keyboard Behavior

The handgrip GPIO shortcut handler still requires both:

- High duration >= 50 ms.
- Low duration >= 50 ms.

After those timing checks, it routes the action instead of calling
`treadmill_key_press()` directly.

The handgrip speed source maps:

- Speed plus GPIO to `KEY_EXT_KBD_SPEED_INC`.
- Speed minus GPIO to `KEY_EXT_KBD_SPEED_DEC`.

The handgrip start/stop source maps:

- Start GPIO to `KEY_EXT_KBD_START`.
- Stop GPIO to `KEY_EXT_KBD_STOP`.

`KEY_EXT_KBD_SPEED_INC` keeps the previous 1 second throttle. The decrement path does
not have that throttle.

Continuous held handgrip keys are monitored by `keyboard_route_key_hold()` so stuck
faults can be detected even when no new edge event is generated.

Release is tracked per handgrip source. When both pins for a handgrip source return
to released state, `keyboard_route_key_release()` is called for that source.

## Handgrip Start/Stop Config Combo

The current branch adds a start-plus-stop combo for the handgrip start/stop source.

Behavior:

- If handgrip START and STOP are both held, the individual START/STOP actions are
  suppressed.
- After 2000 ms of holding both, `KEY_CONFIG` is routed from
  `KEYBOARD_INPUT_SOURCE_HG_START_STOP`.
- The combo emits CONFIG once per hold.
- When the combo is released, the handgrip start/stop source is released and its
  shortcut state is cleared.

## Release-Based Dispatch

The current branch intentionally delays some actions until release:

- Main matrix speed shortcuts: `KEY_SPEED_6`, `KEY_SPEED_9`, `KEY_SPEED_12`,
  `KEY_SPEED_15`.
- Main I2C speed shortcuts: `KEY_EXT_KBD_SPEED_6`, `KEY_EXT_KBD_SPEED_9`,
  `KEY_EXT_KBD_SPEED_12`, `KEY_EXT_KBD_SPEED_15`.
- START keys from main matrix, main I2C, and handgrip start/stop.

This means press/hold is used for health tracking first, and the action is applied
only after the user releases the key.

There is one speed-shortcut exception when C2V is disabled:

- In `SCENE_TRAINING`, if the selected shortcut target is lower than the current
  training speed, the shortcut is allowed on press instead of release.
- Repeated events for the same held shortcut are ignored after the first dispatch.

## Long-Press Ignore for Speed Shortcuts

When all of the following are true:

- Scene is `SCENE_TRAINING`.
- C2V is disabled.
- The key is a speed shortcut release action.
- Held time is >= 3000 ms.

Then release does not apply the shortcut. This avoids long-held shortcut keys becoming
late speed changes after the user releases them.

If the hold reaches the stuck threshold instead, the stuck-key path takes precedence.

## C2V Speed Shortcut Confirmation

C2V is a persisted config option stored in NVM at `NVM_U8_C2V`.

The config option is exposed as `CONFIG_OPTION_C2V`:

- Dot matrix label: `C2F`.
- Seven-segment pace field: current value `0` or `1`.
- Speed plus/minus toggles the value.

When C2V is enabled, speed shortcut increases during training require START
confirmation:

1. User presses and releases a direct speed shortcut for 6, 9, 12, or 15 km/h.
2. If the shortcut target is greater than the current speed, the router does not apply
   the speed immediately.
3. The router stores the pending shortcut and starts a 6000 ms confirmation window.
4. Dot matrix message shows `START`.
5. Seven-segment speed field shows the pending target speed and blinks every 500 ms.
6. If START is released within the window, the stored shortcut key is applied.
7. If the window expires, C2V is disabled, or the scene leaves training, the pending
   shortcut is cleared.

Shortcuts whose target is equal to or lower than the current speed do not require C2V
confirmation and are dispatched normally.

Selecting another C2V shortcut while one is pending replaces/cancels the previous
pending selection.

## Training Speed Key Behavior

Training key behavior after routing reaches `scene_training.c`:

- Main speed plus uses `treadmill_speed_increment()` and therefore follows
  `config.speed.step`.
- Main speed minus uses `treadmill_speed_decrement()` and therefore follows
  `config.speed.step`.
- Handgrip speed plus (`KEY_EXT_KBD_SPEED_INC`) increments current speed by 50
  units, clamped to `config.speed.max`.
- Handgrip speed minus (`KEY_EXT_KBD_SPEED_DEC`) decrements current speed by 100
  units, clamped to `config.speed.min`.
- Direct speed shortcut keys set absolute speed:
  - `KEY_SPEED_6` and `KEY_EXT_KBD_SPEED_6`: 600
  - `KEY_SPEED_9` and `KEY_EXT_KBD_SPEED_9`: 900
  - `KEY_SPEED_12` and `KEY_EXT_KBD_SPEED_12`: 1200
  - `KEY_SPEED_15` and `KEY_EXT_KBD_SPEED_15`: 1500

The current branch added support for main matrix direct speed shortcut keys in
training, not only external I2C shortcut keys.

For WALK_30MIN and WALK_08MIN programs, speed plus/minus updates the program target
speed instead of the current training speed, following the existing program-specific
logic.

## Keyboard Fault Detection

### Constants

| Constant | Value | Meaning |
| --- | --- | --- |
| `KEYBOARD_STUCK_THRESHOLD_MS` | 8000 ms | Held key duration that becomes a stuck fault |
| `MAIN_BURST_WINDOW_MS` | 1000 ms | Burst window for main keyboard source group |
| `MAIN_BURST_MAX` | 8 | More than 8 new presses in the window becomes a main burst fault |
| `HG_BURST_WINDOW_MS` | 1000 ms | Burst window for handgrip groups |
| `HG_BURST_MAX` | 8 | More than 8 new presses in the window becomes a handgrip burst fault |
| `KEYBOARD_SHORTCUT_LONG_PRESS_IGNORE_MS` | 3000 ms | Training speed shortcut long press ignored on release when C2V is disabled |
| `KEYBOARD_FAULT_CLEAR_HOLD_MS` | 5000 ms | Hold time for idle speed plus/minus clear combo |
| `C2V_CONFIRM_WINDOW_MS` | 6000 ms | START confirmation window for C2V speed shortcut increases |

### When Stuck Detection Runs

START keys are always checked for stuck behavior.

In `SCENE_IDLE`, speed shortcut keys are checked:

- Main matrix direct speed shortcuts.
- Main I2C direct speed shortcuts.
- Handgrip speed plus/minus.

In `SCENE_TRAINING`, when C2V is disabled, all non-`KEY_NONE` keys are checked.

In `SCENE_TRAINING`, when C2V is enabled, the current code checks START keys for stuck
behavior but does not apply the all-key training stuck rule.

### Stuck Fault Flow

If a monitored key remains held for at least 8000 ms:

1. The router sets the disabled-source mask for that key source group.
2. The router registers the corresponding keyboard stuck error code.
3. The original key action is not dispatched.
4. Future keys from that source group are blocked except for safety/maintenance
   exceptions.

Stuck faults can be detected:

- During repeated routed key events.
- During `keyboard_route_key_hold()` for handgrip keys.
- On release, if the held duration already exceeded the threshold.

### Burst Fault Flow

On each new press for a source:

1. If the burst window is empty or expired, a new 1000 ms window starts with count 1.
2. Otherwise the count increments.
3. If count becomes greater than 8, the source group is disabled.
4. The corresponding burst error code is registered.
5. The key that caused the burst is not dispatched.

Burst detection is source-specific but latching is by source group. For example, a
burst from main I2C disables the main group, which also blocks main matrix keys except
for safety/maintenance exceptions.

## Keyboard Fault Codes

| Code | Symbol | Meaning | Source disabled |
| --- | --- | --- | --- |
| `0xA1` | `KEYBOARD_ERROR_MAIN_STUCK` | Main keyboard stuck key | Main |
| `0xA2` | `KEYBOARD_ERROR_HG_START_STOP_STUCK` | Handgrip start/stop stuck key | HG start/stop |
| `0xA3` | `KEYBOARD_ERROR_HG_SPEED_STUCK` | Handgrip speed stuck key | HG speed |
| `0xA4` | `KEYBOARD_ERROR_MAIN_BURST` | Main keyboard burst/noise | Main |
| `0xA5` | `KEYBOARD_ERROR_HG_START_STOP_BURST` | Handgrip start/stop burst/noise | HG start/stop |
| `0xA6` | `KEYBOARD_ERROR_HG_SPEED_BURST` | Handgrip speed burst/noise | HG speed |

Keyboard count storage order is:

1. Main stuck.
2. Main burst.
3. Handgrip speed stuck.
4. Handgrip speed burst.
5. Handgrip start/stop stuck.
6. Handgrip start/stop burst.

That order is used by the config ERR browser and NVM keyboard count addresses.

## Source Lockout Rules

When a source group is disabled by a keyboard fault:

- Normal keys from that group are ignored.
- STOP is always allowed.
- CONFIG is always allowed.
- In CONFIG scene, START can be used to navigate as long as the current START key is
  not itself stuck.
- In CONFIG/ERR, principal `KEY_SPEED_INCREMENT` is allowed to browse stored errors
  during maintenance.

These exceptions are safety and service paths so the user can stop, enter config, view
errors, or reset errors even after a keyboard fault latch.

## Keyboard Fault Clear/Unlock Combo

Keyboard fault clear/unlock is available only in `SCENE_IDLE`.

Any of the following can activate the combo:

- Main matrix speed plus and speed minus held together.
- Handgrip speed plus and speed minus held together.
- Main I2C speed plus and speed minus detected within 150 ms.

While the combo is active:

- Fault detection for the combo plus/minus keys is suppressed.
- Source health for the involved source is reset so the clear combo does not create a
  stuck or burst fault.

If the combo remains active for at least 5000 ms:

1. `treadmill_clear_keyboard_error_code_and_unlock()` clears keyboard last-error
   history in RAM and NVM.
2. Keyboard error timestamps are cleared.
3. `disabled_sources_mask` is cleared.
4. Keyboard dirty/save timing state is reset.
5. Main matrix, main I2C, and handgrip speed health state are reset.
6. The treadmill emits `BEEP_KEYPRESS_ACK`.

This clear/unlock path does not reset keyboard error counters. Full counter reset is
done through CONFIG/ERR reset.

## Persistent Error Storage

The current branch extends NVM error storage:

| Address symbol | Purpose |
| --- | --- |
| `NVM_U16_INVERTER_ERROR_0_COUNT` through `NVM_U16_INVERTER_ERROR_8_COUNT` | 9 inverter error counters |
| `NVM_U8_INVERTER_LAST_ERROR_0` through `NVM_U8_INVERTER_LAST_ERROR_2` | Last 3 inverter error codes |
| `NVM_U16_KEYBOARD_ERROR_0_COUNT` through `NVM_U16_KEYBOARD_ERROR_5_COUNT` | 6 keyboard error counters |
| `NVM_U8_KEYBOARD_LAST_ERROR_0` through `NVM_U8_KEYBOARD_LAST_ERROR_2` | Last 3 keyboard error codes |
| `NVM_U8_C2V` | C2V config value |

On startup, `treadmill_load_errors()` loads inverter counts/history and keyboard
counts/history. It rebuilds `disabled_sources_mask` from the persisted keyboard last
errors, so a keyboard fault can remain locked after reboot until the keyboard
clear/unlock combo or a full error reset clears it.

`treadmill_register_keyboard_error()`:

- Does nothing in CONFIG scene.
- Updates the 3-entry keyboard last-error history.
- Updates the per-code count, saturating at `0xffff`.
- Stores an in-RAM timestamp used for latest-error selection.
- Saves immediately if the keyboard save debounce permits it.
- Otherwise marks keyboard errors dirty for a later save.

`treadmill_update()` flushes dirty inverter or keyboard errors once their 5000 ms save
debounce window has elapsed.

`treadmill_get_latest_error_code()` returns the most recent timestamped inverter or
keyboard error. If no timestamp exists after load, it prefers keyboard last error 0
and then inverter last error 0.

## Inverter Error Handling Changes

The inverter error table was expanded from 8 to 9 entries.

New/updated inverter code list:

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

`INVERTER_ERROR_OVERLOAD` was inserted at index 5. Communication, incline motor, and
MPHV shifted to indexes 6, 7, and 8.

Inverter errors continue to be registered through
`treadmill_motor_register_inverter_error()`. Repeated identical errors are suppressed
when the same error arrives within 10 seconds of `last_ack_time`. Counts saturate at
255 in the current inverter update helper.

## Config ERR Screen Behavior

`CONFIG_OPTION_ERRORS` now browses `CONFIG_ERRORS_TOTAL_ENTRIES` entries:

```text
INVERTER_ERROR_COUNT + LAST_ERROR_HISTORY_COUNT + KEYBOARD_ERROR_COUNT
= 9 + 3 + 6
= 18 entries
```

Navigation:

- START / external START moves to the next config option.
- STOP / external STOP exits config and resets `config_error_index` to 0.
- Speed plus advances to the next ERR entry and wraps at the end.
- KEY_CONFIG while on ERR resets all stored inverter and keyboard errors, resets the
  dot-matrix frame messages, shows the reset message for 5000 ms, and returns to
  idle.

Dot matrix display:

- Shows the selected error code as `E*XX`.
- For inverter count entries, `XX` is the inverter error code.
- For inverter last-error entries, `XX` is the stored last inverter error code.
- For keyboard count entries, `XX` is the keyboard error code.

Seven-segment display:

- For inverter count entries, pace shows that inverter counter.
- For inverter last-error entries, pace shows `L01`, `L02`, or `L03`.
- For keyboard count entries, pace shows that keyboard counter.

## Idle Error Display

In `SCENE_IDLE`, the current branch renders keyboard errors in the seven-segment
program field:

- If the latest error code is in `0xA1..0xA6`, program display is visible.
- Text format is space + `E` + two hex characters, for example ` EA1`.
- If the latest error is not a keyboard error, the program field is hidden.

This means idle display focuses on keyboard lock/fault errors. Inverter errors are
still stored and browsed in CONFIG/ERR, but the idle seven-segment error path only
renders keyboard error codes.

## Config and Test Related Changes

The config option list now includes C2V before ERR.

The TEST option behavior changed:

- Toggling TEST on enters `SCENE_TEST`.
- `SCENE_TEST` shows test output on displays.
- START / external START starts the training countdown.
- STOP / external STOP disables test mode and returns to the previous scene.

During training, if test mode remains enabled, the training update loop runs
`update_test_program()` once per second. It cycles through programmed speeds and
inclines for test execution, then disables test mode and stops or pauses at the end.

## User-Facing Behavior Matrix

| Situation | Behavior |
| --- | --- |
| Main/I2C START press | Tracked on press, action applied on release |
| Handgrip START press | Tracked on press, action applied on release |
| Direct speed shortcut 6/9/12/15 in training with C2V disabled | Usually applied on release; if target speed is lower than current speed, applied on press |
| Direct speed shortcut increase in training with C2V enabled | Shows pending target and START prompt, requires START release within 6 seconds |
| Direct speed shortcut decrease/equal in training with C2V enabled | Applied normally without confirmation |
| Speed shortcut held >= 3 seconds in training with C2V disabled | Ignored on release unless it reaches stuck threshold |
| Monitored key held >= 8 seconds | Source group is locked and stuck fault code is stored |
| More than 8 new presses in 1 second for a source | Source group is locked and burst fault code is stored |
| Faulted source group receives normal key | Ignored |
| Faulted source group receives STOP or CONFIG | Allowed |
| Faulted source in CONFIG/ERR receives main speed plus | Allowed to browse ERR entries |
| Idle plus/minus clear combo held 5 seconds | Clears keyboard last errors and unlocks keyboard source groups |
| CONFIG/ERR plus | Advances through 18 error entries |
| CONFIG/ERR CONFIG | Full error reset and return to idle |

## Implementation Source Map

Primary files changed for keyboard and error behavior:

- `rt_led/Core/Inc/main.h`: input source enum and router prototypes.
- `rt_led/Core/Src/main.c`: central keyboard router, source health, stuck/burst
  detection, C2V confirmation, handgrip combo, clear/unlock combo, and matrix routing.
- `rt_led/Core/Src/ext_i2c_kbd.c`: I2C key routing, release events, clear-combo
  tracking, and I2C shortcut re-entry suppression.
- `rt_led/Core/Src/ext_i2c_kbd.h`: clear-combo status export.
- `shared/treadmill.h`: keyboard error codes, storage constants, C2V config, keyboard
  hardware state, and error API declarations.
- `shared/treadmill.c`: keyboard error registration, latest-error selection,
  persistent save/load/reset, and source lock restoration from NVM.
- `shared/treadmill_motor.c`: inverter overload error and expanded inverter error
  index mapping.
- `shared/sd/sd_render.c`: idle keyboard error display, C2V speed feedback, C2V config
  value display, and ERR counter browsing.
- `shared/dm/dm_render.c`: C2V config label and `E*XX` ERR code display for inverter,
  inverter last-error, and keyboard entries.
- `shared/scene/scene_config.c`: C2V toggle, expanded ERR browsing, and CONFIG-based
  full error reset from ERR.
- `shared/scene/scene_training.c`: main shortcut speed keys, external speed step
  behavior, test-mode stop handling, and test-program update.
- `shared/scene/scene_test.c`: added test scene key behavior.
- `shared/panel/rt_led.c` and `shared/panel/rt_led.h`: firmware version bump,
  C2V config option, test scene registration, and external START no-beep countdown
  handling.

## Comparison Against Aria-1.1.3-shortcut-kbd

The baseline branch had:

- Direct `treadmill_key_press()` calls from main matrix, I2C keyboard, and handgrip
  shortcut handling.
- No source-aware keyboard router.
- No keyboard stuck or burst fault codes.
- No keyboard source lockout.
- No persistent keyboard error counters or keyboard last-error history.
- No C2V option or START confirmation for shortcut speed increases.
- ERR config browsing limited to inverter counters plus last inverter errors.
- Inverter error storage with 8 counters and no overload entry.
- Handgrip start/stop GPIOs dispatching `KEY_START` and `KEY_STOP` directly.

The current branch adds:

- Central routing for all keyboard sources.
- Press/hold/release semantics.
- START and shortcut speed release dispatch.
- C2V speed-shortcut confirmation.
- Handgrip start+stop config combo.
- Keyboard stuck and burst detection.
- Source lockout and persisted lock restoration.
- Idle keyboard fault display.
- 18-entry ERR browser across inverter counts, inverter last errors, and keyboard
  counts.
- Full error reset from CONFIG/ERR with user-facing reset message.
- Inverter overload error code and 9-entry inverter storage.

## Notes and Risks

- The C2V config label is rendered as `C2F` in the current dot-matrix code. This
  document records the implemented label rather than renaming it.
- Main matrix and main I2C are deliberately grouped under one main-source lockout in
  the implementation. A fault from either main source disables both, except for the
  documented safety/maintenance exceptions.
- I2C key `0x09` now routes to `KEY_STOP`; the baseline routed this key to
  `KEY_EXT_KBD_STOP`.
- This document was verified against source code and branch diff only. Timing,
  simultaneous key behavior, and NVM persistence should still be validated on the
  target hardware.
