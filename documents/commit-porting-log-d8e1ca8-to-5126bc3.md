# Porting Log: d8e1ca8 Through 5126bc3

## Scope

This document explains the commits from:

```text
d8e1ca80757fbb666763fe518149c47c2eedf91f^..5126bc38ee0c3812c3941cb4f7fd53e611a948e0
```

That is the series from `WIP 124 - starting point` through current HEAD
`Aria-1.2.4---ext-kbd-board-and-errors`.

Purpose:

- Explain what each commit changed.
- Describe how system behavior was impacted.
- Highlight dependencies and risks for porting these changes to another branch.

This is a source-derived analysis. It was not validated on hardware while writing this
document.

## Range Summary

| Metric | Value |
| --- | --- |
| First included commit | `d8e1ca80757fbb666763fe518149c47c2eedf91f` |
| Baseline parent | `ec8037fae655d23b07b18fd1cc61dc945d02ab78` |
| Last included commit | `5126bc38ee0c3812c3941cb4f7fd53e611a948e0` |
| Commits included | 11 |
| Files changed | 25 |
| Net diff | 1931 insertions, 289 deletions |

## Main Behavioral Themes

The commit range implements five major themes:

1. Firmware/version and UI expansion for 1.2.4/1.2.5.
2. Safer display formatting with fewer `snprintf()` paths in embedded display code.
3. Expanded inverter error tracking, including overload.
4. New keyboard error architecture: source routing, stuck/burst detection, source
   lockout, NVM persistence, display, and clearing.
5. C2V speed-shortcut confirmation behavior, with later tuning of timing and trigger
   rules.

## Porting Strategy

For a clean port, do not cherry-pick only the final keyboard router without its data
model and display support. The final behavior depends on changes across:

- `rt_led/Core/Inc/main.h`
- `rt_led/Core/Src/main.c`
- `rt_led/Core/Src/ext_i2c_kbd.c`
- `rt_led/Core/Src/ext_i2c_kbd.h`
- `shared/treadmill.h`
- `shared/treadmill.c`
- `shared/sd/sd_render.c`
- `shared/dm/dm_render.c`
- `shared/scene/scene_config.c`
- `shared/panel/rt_led.h`

Recommended porting order:

1. Port structural model changes first: scenes, constants, error enums, NVM addresses,
   `treadmill_t` fields.
2. Port persistence functions: save/load/reset keyboard and inverter errors.
3. Port router API declarations and key-source enum.
4. Port input source changes: main matrix, I2C keyboard, handgrip GPIOs.
5. Port scene/config/display changes.
6. Apply the follow-up tuning commits in order, especially threshold and error-code
   corrections.

## Commit Timeline

| Commit | Date | Short message | Primary impact |
| --- | --- | --- | --- |
| `d8e1ca8` | 2026-04-17 10:35 -0300 | WIP 124 - starting point | Establishes 1.2.4 base: test scene, overload error, display formatting, fixed-point display conversion |
| `094dd0b` | 2026-04-17 18:08 -0300 | WIP 124 - TODO: improve clearing sd_render error via plus/minus keys | Adds first keyboard router, keyboard error persistence, C2V config, ERR browser expansion, release-based actions |
| `9f3693d` | 2026-04-17 18:54 -0300 | WIP 124 - test | Adds C2V user feedback on dot matrix and seven-segment speed display |
| `9b23d0b` | 2026-04-17 18:58 -0300 | Enhance C2V shortcut handling in training mode | Restricts C2V confirmation to speed increases and clears pending shortcuts on direct selection |
| `bae88ab` | 2026-04-21 08:39 -0300 | WIP 124 - removed inverter errors on sd_render | Limits idle seven-segment error display to keyboard errors only |
| `67b41c9` | 2026-04-21 09:40 -0300 | WIP1.2.5 - ready to test | Adds I2C plus/minus clear combo and suppresses faults during clear combo |
| `85e45b0` | 2026-04-21 09:41 -0300 | WIP 1.2.5 - ready to test; fixed version number | Bumps firmware revision from 1.2.4 to 1.2.5 |
| `a6b3aa4` | 2026-04-21 13:09 -0300 | WIP 1.2.5 - changed trigger times for stuck keys and reset error message key combination | Moves timing constants to shared header and changes stuck/clear/C2V timings |
| `36a711b` | 2026-04-21 15:26 -0300 | Enhance keyboard stuck key detection for training mode | Extends stuck detection into training when C2V is disabled |
| `a7e59ad` | 2026-04-21 15:53 -0300 | WIP 1.2.5 - added long press ignore for speed shortcut keys | Adds 3-second long-press ignore for speed shortcut release behavior |
| `5126bc3` | 2026-04-21 17:31 -0300 | WIP 1.2.5 - adjusted after feedback - now start getting stuck is returning error | Makes START keys stuck-detectable, fixes keyboard error code ordering, adds I2C shortcut re-entry filter |

## Detailed Commit Log

### d8e1ca8 - WIP 124 - Starting Point

Files changed:

- `rt_led/Core/Src/ext_i2c_kbd.c`
- `shared/dm/dm_render.c`
- `shared/lang.c`
- `shared/lang.h`
- `shared/modbus/master_operations.c`
- `shared/panel/render_program_scene.c`
- `shared/panel/rt_led.c`
- `shared/panel/rt_led.h`
- `shared/scene/scene_config.c`
- `shared/scene/scene_test.c`
- `shared/scene/scene_training.c`
- `shared/sd/sd.c`
- `shared/sd/sd_render.c`
- `shared/treadmill.c`
- `shared/treadmill.h`
- `shared/treadmill_action.c`
- `shared/treadmill_ble.c`
- `shared/treadmill_motor.c`
- `shared/treadmill_programs.h`

What changed:

- Firmware version changed from `1.1.4` to `1.2.4`.
- Added `SCENE_TEST` to the scene enum, scene-name table, and scene registry.
- Added `shared/scene/scene_test.c`.
- Config TEST option now enters `SCENE_TEST` when enabled.
- `SCENE_TEST` shows `START`, START begins countdown, STOP disables test and returns
  to the previous scene.
- Training gained a test-program cycle that steps speed/incline through test values
  while `config.test` is enabled.
- Added inverter overload error:
  - `INVERTER_ERROR_OVERLOAD = 0x40`
  - Added ninth inverter error NVM counter.
  - Shifted communication/incline/MPHV indexes.
- Added language strings for overload and reset/apply messaging:
  - `STR_ERR_OVERLOAD`
  - `STR_TM_RESET`
- Changed inverter error display support in dot matrix:
  - Added overload to displayed error code list.
  - Last-error display changed from `L*X` style to `E*X` style in this starting
    implementation.
- Added safer integer-based formatting helpers in several display-related modules:
  - `shared/sd/sd.c`
  - `shared/dm/dm_render.c`
  - `shared/modbus/master_operations.c`
  - `shared/panel/render_program_scene.c`
  - `shared/treadmill_ble.c`
- Replaced many `snprintf()` display paths with bounded integer/string formatting.
- Changed `treadmill_calculate_pace()` to integer math and added speed-zero guards.
- Added support for negative incline text formatting in dot-matrix incline display.
- Added idle seven-segment rendering of the latest inverter error in `sd_render`.
- Split idle and BLE-allow display handling in `sd_render`.
- Added all-8888 seven-segment render in `SCENE_TEST`.
- Updated HIIT time constants from floating macro expressions to integer `30`.
- Changed BLE command formatting to avoid `snprintf()` when building device name and
  firmware version commands.
- Added `treadmill_config()` behavior to clear `config.test` when entering config.
- Updated I2C CONFIG key handling: key `0x0F` can emit CONFIG inside CONFIG after the
  scene has been active for more than 1000 ms.

Behavior impact:

- The firmware reports version 1.2.4.
- Users can enter a test scene from the TEST config option and start a test training
  cycle.
- The inverter error model now includes overload and expects 9 counters instead of 8.
- Display formatting is less dependent on stdio formatting, which is useful for
  embedded footprint and deterministic formatting.
- Reported/displayed pace and unit conversions use integer math.
- Idle display can show an inverter error code on the program field.
- I2C CONFIG key can operate again while already in CONFIG, after a one-second guard.

Porting notes:

- Port `INVERTER_ERROR_COUNT` and all downstream index logic together. Adding overload
  without updating save/load/display arrays will misalign error counters.
- Port `SCENE_TEST` enum, scene registry, and `scene_name[]` together. A missing scene
  registry entry will make the scene unreachable or unhandled.
- Check NVM compatibility: adding `NVM_U16_INVERTER_ERROR_8_COUNT` shifts later
  addresses once keyboard storage is added in `094dd0b`.
- The fixed-format display helpers are mechanical but broad. When porting to a branch
  that still uses `snprintf()`, choose either full helper adoption or carefully keep
  existing stdio behavior consistent.

### 094dd0b - Initial Keyboard Error Architecture

Files changed:

- `rt_led/Core/Inc/main.h`
- `rt_led/Core/Src/ext_i2c_kbd.c`
- `rt_led/Core/Src/main.c`
- `shared/dm/dm_render.c`
- `shared/panel/rt_led.c`
- `shared/panel/rt_led.h`
- `shared/scene/scene_ble_allow_yes_no.c`
- `shared/scene/scene_config.c`
- `shared/scene/scene_idle.c`
- `shared/scene/scene_test.c`
- `shared/sd/sd_render.c`
- `shared/treadmill.c`
- `shared/treadmill.h`
- `shared/treadmill_programs.c`

What changed:

- Introduced `keyboard_input_source_t`:
  - Main matrix
  - Main I2C
  - Handgrip speed
  - Handgrip start/stop
- Declared router functions:
  - `keyboard_route_key_event()`
  - `keyboard_route_key_release()`
- Converted external I2C keyboard handling from direct `treadmill_key_press()` calls
  to router calls.
- Converted main matrix handling from direct keypress calls to router calls.
- Converted handgrip GPIO shortcut handling to router calls.
- Added central keyboard router in `main.c`.
- Added per-source health state:
  - current pressed state
  - last key
  - press start time
  - burst window time
  - burst count
- Added source groups:
  - Main
  - Handgrip speed
  - Handgrip start/stop
- Added source lockout masks:
  - Main
  - Handgrip speed
  - Handgrip start/stop
- Added keyboard stuck and burst fault mapping.
- Initial timing in this commit:
  - Main stuck: 2000 ms
  - Handgrip stuck: 2000 ms
  - Burst window: 1000 ms
  - Burst max: 8
  - C2V window: 3000 ms
  - Keyboard clear combo hold: 10000 ms
- Added release-based dispatch for:
  - START
  - Direct speed shortcut keys
- Added first C2V pending shortcut implementation.
- Added first handgrip START+STOP combo:
  - hold 2000 ms to emit CONFIG.
- Added first idle plus/minus clear combo using main matrix or handgrip speed combo.
- Added keyboard error data model to `treadmill_t`.
- Added keyboard error NVM storage:
  - 6 keyboard counters.
  - 3 keyboard last-error slots.
- Added `treadmill_register_keyboard_error()`.
- Added `treadmill_get_latest_error_code()`.
- Added `treadmill_clear_keyboard_error_code_and_unlock()`.
- Added dirty/debounced save handling in `treadmill_update()`.
- Added loading of keyboard errors and restoration of keyboard source lockout from
  persisted keyboard last errors.
- Added C2V config option:
  - `CONFIG_OPTION_C2V`
  - `RT_LED_CONFIG_C2V_*`
  - `config.c2v`
  - `NVM_U8_C2V`
- Expanded CONFIG/ERR display:
  - Inverter counters.
  - Inverter last errors.
  - Keyboard counters.
- Updated dot matrix ERR rendering to use a helper that maps config error index to
  the displayed code.
- Updated seven-segment ERR display to show `L01`, `L02`, `L03` for inverter
  last-error slots.
- Added reset behavior in CONFIG/ERR:
  - Press CONFIG on ERR to call `treadmill_reset_errors()`.
  - Show reset message.
  - Return idle.
- Added external START handling to more scenes/program handlers:
  - BLE allow yes/no scene.
  - Test scene.
  - Program handlers.
- Updated `rt_led_key_press()` so external START from idle also suppresses the normal
  key beep before countdown.
- Added main direct speed shortcut handling in training:
  - `KEY_SPEED_6`, `KEY_SPEED_9`, `KEY_SPEED_12`, `KEY_SPEED_15`.
- Added idle `KEY_EXT_KBD_START` handling.

Behavior impact:

- Keyboard sources are no longer raw key-to-action paths. They pass through safety
  checks and release semantics first.
- A source can be disabled by stuck/burst detection.
- STOP and CONFIG remain available as safety/service exceptions even when a source is
  locked.
- Keyboard faults can persist across reboot through NVM last-error restoration.
- Config ERR becomes the central service screen for inverter and keyboard error
  history.
- Direct speed shortcut and START behavior becomes release-based rather than
  immediate in selected paths.
- C2V mode can require START confirmation for speed shortcut use.

Porting notes:

- This commit is the architectural center of the keyboard work. Do not port later
  threshold commits without this one.
- If porting to a branch with different NVM usage, review the new addresses
  `0x0019..0x0022`.
- The keyboard error code order in this commit is later corrected in `5126bc3`. Do not
  preserve the early order as final behavior.
- The initial timing values are later changed in `a6b3aa4`; port final constants, not
  the initial local `MAIN_STUCK_MS`/`HG_STUCK_MS` values.

### 9f3693d - C2V Feedback UI

Files changed:

- `rt_led/Core/Src/main.c`
- `shared/sd/sd_render.c`
- `shared/treadmill.h`

What changed:

- Moved `C2V_CONFIRM_WINDOW_MS` to `shared/treadmill.h`.
- Added C2V feedback fields to `treadmill->aux`:
  - `c2v_shortcut_sd_feedback_active`
  - `c2v_shortcut_sd_feedback_start_ms`
  - `c2v_shortcut_sd_feedback_speed`
- Added `g_c2v_start_prompt_active` in the keyboard router.
- Added dot-matrix prompt management:
  - Shows `START` while waiting for confirmation.
  - Clears `START` if the pending prompt is still active when C2V state clears.
- Added seven-segment speed feedback during pending C2V:
  - Shows selected target speed.
  - Blinks every 500 ms.
  - Expires with the C2V window.

Behavior impact:

- C2V is now visible to the user. A pending shortcut no longer silently waits for
  START; it prompts START and displays/blinks the pending target speed.
- Expiring/canceling C2V now clears both prompt and seven-segment feedback state.

Porting notes:

- Port `treadmill->aux` fields with `sd_render` changes. If fields are omitted,
  C2V may still change behavior but user feedback will be broken.
- `main.c` now depends on `treadmill_set_dm_message()` and message text comparison for
  prompt lifecycle.

### 9b23d0b - C2V Only For Speed Increases

Files changed:

- `rt_led/Core/Src/main.c`

What changed:

- Changed C2V shortcut gating from "any direct speed shortcut in training" to "only
  direct speed shortcut targets greater than current speed".
- If a direct shortcut is selected while another C2V shortcut is pending, the old
  confirmation window is cleared.

Behavior impact:

- Speed reductions or same-speed shortcut selections do not require START
  confirmation.
- C2V specifically protects upward speed jumps.
- A user can replace/cancel an old pending C2V shortcut by selecting another shortcut.

Porting notes:

- Keep this with C2V support. Without it, C2V may incorrectly require confirmation for
  speed reductions, which is not final behavior.

### bae88ab - Idle Display Shows Keyboard Errors Only

Files changed:

- `shared/sd/sd_render.c`

What changed:

- Added `sd_is_keyboard_error_code()`.
- Changed idle error display to hide non-keyboard errors.
- The idle seven-segment program field only shows errors in keyboard range
  `0xA1..0xA6`.

Behavior impact:

- Inverter errors are still saved and browseable in CONFIG/ERR, but idle display no
  longer shows inverter error codes.
- Idle error display is now focused on keyboard lock/fault recovery.

Porting notes:

- If the target branch expects inverter errors on idle seven-segment display, this is
  a deliberate behavior change to review with product/service stakeholders.

### 67b41c9 - I2C Clear Combo and Clear-Combo Fault Suppression

Files changed:

- `rt_led/Core/Src/ext_i2c_kbd.c`
- `rt_led/Core/Src/ext_i2c_kbd.h`
- `rt_led/Core/Src/main.c`

What changed:

- Added I2C speed plus/minus clear combo tracking:
  - `EXT_I2C_CLEAR_COMBO_WINDOW_MS = 150`
  - Tracks last speed increment/decrement tick.
  - Marks combo active when plus/minus are seen within 150 ms.
- Exported `ext_i2c_kbd_is_clear_combo_pressed()`.
- Resets I2C clear-combo state on I2C keyboard init and key release.
- Main keyboard clear combo now considers:
  - Main matrix plus/minus combo.
  - Handgrip speed plus/minus combo.
  - I2C plus/minus combo.
- Added `keyboard_reset_source_health()`.
- Added clear-combo active/key checks in router.
- Suppresses stuck/burst fault creation while the clear combo is active for the
  participating plus/minus key.
- After successful unlock, resets health for:
  - Main matrix
  - Main I2C
  - Handgrip speed

Behavior impact:

- Users can clear/unlock keyboard faults from the I2C keyboard, not only physical
  matrix or handgrip speed controls.
- Holding the clear combo no longer creates a new stuck/burst fault while trying to
  clear an old one.
- Router health is reset after unlock to avoid stale press state.

Porting notes:

- Port `ext_i2c_kbd.h` changes with `main.c`; otherwise `main.c` will miss the I2C
  combo accessor.
- This commit assumes I2C plus/minus are represented as `KEY_SPEED_INCREMENT` and
  `KEY_SPEED_DECREMENT`, not external speed increment/decrement aliases.

### 85e45b0 - Version Bump To 1.2.5

Files changed:

- `shared/panel/rt_led.h`

What changed:

- Firmware revision changed from `1.2.4` to `1.2.5`.

Behavior impact:

- Reported firmware version becomes 1.2.5.
- BLE firmware version reporting also changes because it derives from `fw_version`.

Porting notes:

- Apply only if the port should identify itself as 1.2.5. If the target branch has
  its own versioning scheme, map this to that scheme instead of blindly copying.

### a6b3aa4 - Final Timing Constants For 1.2.5

Files changed:

- `rt_led/Core/Src/main.c`
- `shared/treadmill.h`

What changed:

- Removed local `MAIN_STUCK_MS` and `HG_STUCK_MS` constants from `main.c`.
- Added shared timing constants:
  - `KEYBOARD_STUCK_THRESHOLD_MS = 8000`
  - `KEYBOARD_FAULT_CLEAR_HOLD_MS = 5000`
  - `C2V_CONFIRM_WINDOW_MS = 6000`
- Changed stuck threshold from 2000 ms to 8000 ms.
- Changed keyboard clear combo hold time from 10000 ms to 5000 ms.
- Changed C2V confirmation window from 3000 ms to 6000 ms.
- `keyboard_stuck_threshold_ms()` now returns the shared 8000 ms for all groups.

Behavior impact:

- Stuck detection becomes less aggressive.
- Keyboard unlock combo becomes faster.
- C2V confirmation gives the user more time to press START.

Porting notes:

- These are final timing values for this branch. If porting keyboard router from
  `094dd0b`, overwrite its initial local constants with these shared constants.
- Because `C2V_CONFIRM_WINDOW_MS` is used by display code and router code, keep it in
  the shared header.

### 36a711b - Training Mode Stuck Detection

Files changed:

- `rt_led/Core/Src/main.c`

What changed:

- Expanded `keyboard_should_check_stuck()`:
  - In idle, still checks speed shortcut keys.
  - In training with C2V disabled, checks any non-`KEY_NONE` key.
  - In training with C2V enabled, does not apply the all-key training rule.
- Added stuck check on release before release-action dispatch:
  - If held time has reached stuck threshold, latch source disabled and register stuck
    fault before applying the release action.

Behavior impact:

- Training mode can now detect stuck keys, not just idle speed shortcut stuck keys.
- A key held through the threshold and released will be recorded as stuck rather than
  applying late.
- C2V-enabled training is intentionally narrower here; later `5126bc3` adds START
  stuck detection regardless of C2V.

Porting notes:

- Port this after timing constant changes so training stuck uses 8000 ms, not the
  early 2000 ms value.
- This commit depends on router release-state tracking.

### a7e59ad - Long-Press Ignore For Speed Shortcuts

Files changed:

- `rt_led/Core/Src/main.c`
- `shared/treadmill.h`

What changed:

- Added `KEYBOARD_SHORTCUT_LONG_PRESS_IGNORE_MS = 3000`.
- Changed speed shortcut release handling:
  - In training.
  - C2V disabled.
  - Speed shortcut release behavior.
  - Held time >= 3000 ms.
  - Then clear source press state and do not dispatch the speed shortcut.

Behavior impact:

- A long-held direct speed shortcut no longer applies a delayed speed jump when
  released.
- If the hold reaches the 8000 ms stuck threshold, stuck handling still happens first.

Porting notes:

- Keep this after `36a711b`; the release path first checks stuck threshold, then
  long-press ignore.
- This protects shortcut behavior without disabling the source.

### 5126bc3 - START Stuck Detection, I2C Re-entry Filter, Final Keyboard Codes

Files changed:

- `rt_led/Core/Src/main.c`
- `shared/treadmill.h`

What changed:

- Added `keyboard_is_start_key()`.
- Changed `keyboard_should_check_stuck()` so START keys are always checked for stuck:
  - `KEY_START`
  - `KEY_EXT_KBD_START`
- Added I2C direct-speed shortcut seen-mask:
  - Tracks I2C shortcut 6, 9, 12, 15 during a press.
  - Ignores repeated old shortcut re-entry when I2C reports transient sequences like
    `15 -> 12 -> 15`.
  - Resets the seen mask when a new I2C press begins or I2C source releases.
- Corrected keyboard error code values:
  - `KEYBOARD_ERROR_MAIN_STUCK = 0xA1`
  - `KEYBOARD_ERROR_HG_START_STOP_STUCK = 0xA2`
  - `KEYBOARD_ERROR_HG_SPEED_STUCK = 0xA3`
  - `KEYBOARD_ERROR_MAIN_BURST = 0xA4`
  - `KEYBOARD_ERROR_HG_START_STOP_BURST = 0xA5`
  - `KEYBOARD_ERROR_HG_SPEED_BURST = 0xA6`

Behavior impact:

- START can now produce a stuck-key error. This was the named feedback adjustment in
  the commit message.
- I2C simultaneous/direct shortcut behavior is more stable; final dispatch favors the
  latest distinct shortcut rather than a reappearing older one.
- Keyboard error codes become final and service-visible. Any documentation, QA script,
  or service procedure must use these final values.

Porting notes:

- This is a must-port final correction. Without it, START stuck faults are missed.
- This commit changes keyboard error code meanings from the earlier `094dd0b` order.
  Porters must update display/service expectations and any NVM interpretation.
- If older test units already stored keyboard last errors using the pre-`5126bc3`
  order, consider whether a migration/clear step is needed. The code itself does not
  migrate old stored meanings.

## Cumulative System Behavior After HEAD

### Keyboard Behavior

The system now treats keyboard input as source-aware events:

- Main matrix.
- Main I2C.
- Handgrip speed.
- Handgrip start/stop.

Selected keys are release-based:

- START.
- Direct speed shortcuts.

The router can suppress, delay, confirm, or fault a key before the scene sees it.

### Error Handling

The system now tracks:

- 9 inverter error counters.
- 3 inverter last errors.
- 6 keyboard error counters.
- 3 keyboard last errors.

Keyboard last errors can lock out their source group across reboot. Full reset happens
from CONFIG/ERR. Keyboard last errors can be cleared and keyboard sources unlocked
from idle by holding speed plus/minus for 5 seconds.

### Display Behavior

The final display behavior is:

- Idle seven-segment program field shows keyboard errors only.
- CONFIG/ERR dot matrix shows `E*XX` for inverter count entries, inverter last-error
  entries, and keyboard count entries.
- CONFIG/ERR seven-segment pace field shows inverter counts, `L01..L03` for last
  inverter error slots, and keyboard counts.
- C2V pending speed shortcuts show `START` on dot matrix and blink the selected speed
  on the speed display.

### C2V Behavior

When C2V is enabled in training:

- Direct speed shortcut increases require START confirmation within 6 seconds.
- Direct speed shortcut decreases or equal-speed selections do not require
  confirmation.
- Pending selection clears on timeout, scene change, C2V disable, or direct shortcut
  replacement.

### Timing Behavior

Final timing constants:

| Constant | Value |
| --- | --- |
| `KEYBOARD_STUCK_THRESHOLD_MS` | 8000 ms |
| `KEYBOARD_SHORTCUT_LONG_PRESS_IGNORE_MS` | 3000 ms |
| `KEYBOARD_FAULT_CLEAR_HOLD_MS` | 5000 ms |
| `C2V_CONFIRM_WINDOW_MS` | 6000 ms |
| Burst window | 1000 ms |
| Burst max | >8 new presses in the window |
| Handgrip START+STOP CONFIG hold | 2000 ms |
| I2C plus/minus clear combo window | 150 ms |

## Porting Checklist

### Required Data Model Changes

- Add `SCENE_TEST`.
- Add `config.c2v`.
- Add C2V feedback fields in `struct aux`.
- Expand inverter errors to `INVERTER_ERROR_COUNT = 9`.
- Add keyboard error enum and `keyboard_error_t`.
- Add keyboard hardware state in `treadmill->hw.keyboard`.
- Add final timing constants to `shared/treadmill.h`.
- Add NVM addresses for keyboard counters/history and C2V.

### Required Firmware Input Changes

- Add `keyboard_input_source_t`.
- Add router prototypes to `main.h`.
- Route main matrix keys through `keyboard_route_key_event()` and release through
  `keyboard_route_key_release()`.
- Route I2C keys through the router and emit release on `0xff`.
- Route handgrip keys through the router and emit source releases.
- Add handgrip START+STOP CONFIG combo.
- Add I2C plus/minus clear combo and exported status function.
- Add I2C shortcut seen-mask filtering from `5126bc3`.

### Required Persistence Changes

- Save keyboard counters and last errors.
- Load keyboard counters and last errors.
- Rebuild `disabled_sources_mask` from keyboard last errors.
- Debounce dirty error saves in `treadmill_update()`.
- Add `treadmill_clear_keyboard_error_code_and_unlock()`.
- Update `treadmill_reset_errors()` to reset both inverter and keyboard state.

### Required Display/Scene Changes

- Add `SCENE_TEST` to scene registry.
- Add C2V config option and display.
- Expand CONFIG/ERR browser to 18 entries.
- Add idle keyboard error display behavior.
- Add C2V pending target feedback.
- Add external START support in program handlers and BLE/test scenes.
- Update no-beep logic for idle external START.

## Porting Risks

| Risk | Why it matters | Mitigation |
| --- | --- | --- |
| NVM address collision | New addresses `0x0019..0x0022` may conflict with target branch data | Audit target branch NVM map before porting |
| Error code order drift | `5126bc3` changes keyboard error code meanings from earlier commits | Use final enum values only |
| Partial router port | Input may compile but skip persistence/display or safety exceptions | Port router, data model, and persistence as one unit |
| Timing mismatch | Early commits use 2 s stuck / 10 s clear / 3 s C2V, final branch uses 8 s / 5 s / 6 s | Port final shared constants |
| Old stored keyboard errors | Devices flashed with pre-final enum values may interpret persisted `0xA2..0xA6` differently | Clear keyboard errors during upgrade or add migration if required |
| I2C shortcut behavior | Without `5126bc3` seen-mask, simultaneous shortcut presses may dispatch the wrong target | Include final I2C re-entry filter |
| Display-only assumptions | Idle display intentionally hides inverter errors after `bae88ab` | Confirm product/service expectation before port |
| Desktop simulator mismatch | Desktop input bypasses firmware router | Do not use desktop simulator alone to validate keyboard fault behavior |

## Suggested Hardware Validation

Before accepting a port, validate:

1. Main matrix START release from idle starts countdown and does not double-beep.
2. External START release from idle starts countdown and does not double-beep.
3. Handgrip START release starts/resumes where expected.
4. Main/I2C/handgrip speed plus/minus still adjust speed in training.
5. Direct speed shortcut increases with C2V enabled require START and expire after
   6 seconds.
6. Direct speed shortcut decreases with C2V enabled apply without START.
7. Direct speed shortcut held 3 to 8 seconds in training with C2V disabled is ignored
   on release.
8. START held 8 seconds generates the appropriate stuck error.
9. Training key held 8 seconds with C2V disabled generates stuck error.
10. More than 8 new presses in 1 second generates burst error.
11. Faulted source still allows STOP and CONFIG.
12. Idle plus/minus held 5 seconds clears keyboard last errors and unlocks sources.
13. CONFIG/ERR CONFIG resets all inverter and keyboard counters/history.
14. Reboot after keyboard fault restores the source lockout from NVM.
15. I2C simultaneous direct shortcut sequence dispatches the latest distinct shortcut.

## File-Level Port Map

| File | Port relevance |
| --- | --- |
| `rt_led/Core/Inc/main.h` | Source enum and router function declarations |
| `rt_led/Core/Src/main.c` | Core keyboard router, handgrip handling, matrix routing, C2V confirmation, fault clear |
| `rt_led/Core/Src/ext_i2c_kbd.c` | I2C source routing, release, clear combo tracking |
| `rt_led/Core/Src/ext_i2c_kbd.h` | I2C clear combo exported status |
| `shared/treadmill.h` | Error constants, enums, NVM map, C2V/test/keyboard state |
| `shared/treadmill.c` | Error persistence, config load/save, source lock restoration |
| `shared/treadmill_motor.c` | Inverter overload index mapping |
| `shared/sd/sd.c` | Integer display formatting and unit conversion helpers |
| `shared/sd/sd_render.c` | Idle error display, C2V speed feedback, config ERR display |
| `shared/dm/dm_render.c` | C2V/ERR labels and error code rendering |
| `shared/scene/scene_config.c` | C2V/test/error-reset config behavior |
| `shared/scene/scene_training.c` | Main direct speed shortcuts, test-program cycle |
| `shared/scene/scene_test.c` | New test scene |
| `shared/panel/rt_led.c` | Scene registry and idle external START beep behavior |
| `shared/panel/rt_led.h` | Version, C2V config option |
| `shared/treadmill_programs.c` | External START accepted in program setup |
| `shared/treadmill_ble.c` | BLE command formatting without `snprintf()` |
| `shared/modbus/master_operations.c` | Modbus parameter formatting without `snprintf()` |
| `shared/lang.c`, `shared/lang.h` | New strings for overload/reset/apply messaging |
