# RT G4 LED Firmware

Firmware for the RT G4 LED treadmill panel.

The project contains three build surfaces:

- the main STM32G070 application firmware in `rt_led`
- the STM32G070 bootloader firmware in `rt_led_bootloader`
- the host-side desktop harness in `desktop`, built with CMake for local tests

## Developer Workflow

Use this workflow when changing the firmware or CI/CD scripts.

### 1. Create A Branch

Start from the latest main branch:

```sh
git checkout main
git pull
git checkout -b fix/some-panel-behavior
```

Use a branch name that describes the change, such as:

```text
fix/key-release-handling
feature/bootloader-dfu-progress
chore/tighten-firmware-size-check
```

Branches named `Aria-*` also run CI on push. Use that pattern only for the
project's Aria release or experiment flow.

### 2. Make The Firmware Change

Edit the firmware sources under:

```text
rt_led/Core
rt_led_bootloader/Core
shared
desktop
```

For panel behavior changes, the most common files are:

```text
rt_led/Core/Src/main.c
rt_led/Core/Src/app_freertos.c
rt_led/Core/Src/keyboard.c
rt_led/Core/Src/display.c
shared/panel
shared/sd
```

For bootloader and DFU changes, the most common files are:

```text
rt_led_bootloader/Core/Src/main.c
rt_led_bootloader/Core/Src/ble.c
rt_led_bootloader/Core/Src/display.c
```

If the shared protocol, BLE behavior, keyboard behavior, display rendering, or
flash layout changes, update the related host harness or documentation
deliberately. Do not change release tags or artifact names for ordinary firmware
bug fixes.

### 3. Run Host Tests

Run the host-side CI test contract:

```sh
./scripts/ci_test_host.sh
```

This configures and builds the desktop harness with CMake using
`scripts/build-desktop.sh`, then runs CTest when the build directory contains
tests.

### 4. Run Static Analysis

Run cppcheck through the same wrapper used by CI:

```sh
./scripts/ci_static_analysis.sh
```

This checks `rt_led/Core`, `rt_led_bootloader/Core`, `shared`, and `desktop`.
Cppcheck findings are reported without failing by default. Set
`CPPCHECK_ERROR_EXITCODE=1` only when you intentionally want findings to fail
the run.

### 5. Build The Firmware Locally

For the application firmware, run the local build gate:

```sh
./scripts/ci.sh
```

This initializes submodules, checks that the shared STM32 HAL/CMSIS driver tree
is present, runs host tests, runs static analysis, builds `rt_led` in `Debug`,
and checks firmware size.

The local non-Docker firmware build depends on STM32CubeIDE-generated makefiles
under the selected configuration directory, such as:

```text
rt_led/Debug/makefile
```

### 6. Build With The Same Docker Flow As CI

To match the GitHub Actions firmware build, use the Dockerized STM32CubeIDE
builder:

```sh
./scripts/build-cubeide-docker.sh rt_led Debug 2.1.1
./scripts/check-firmware-size.py rt_led Debug
./scripts/package-firmware-artifacts.sh rt_led Debug
```

For the bootloader:

```sh
./scripts/build-cubeide-docker.sh rt_led_bootloader Debug 2.1.1
./scripts/check-firmware-size.py rt_led_bootloader Debug
./scripts/package-firmware-artifacts.sh rt_led_bootloader Debug
```

The bootloader project uses a CubeIDE linked `Drivers` folder that points to
`rt_led/Drivers`, so a fresh clone does not need a local
`rt_led_bootloader/Drivers` symlink.

This generates firmware outputs in each project's configuration directory, for
example:

```text
rt_led/Debug/rt_led.bin
rt_led/Debug/rt_led.elf
rt_led/Debug/rt_led.map
rt_led/Debug/rt_led.list
rt_led_bootloader/Debug/rt_led_bootloader.bin
rt_led_bootloader/Debug/rt_led_bootloader.elf
rt_led_bootloader/Debug/rt_led_bootloader.map
rt_led_bootloader/Debug/rt_led_bootloader.list
```

Packaged artifacts are copied to:

```text
build/firmware/rt_led/
build/firmware/rt_led_bootloader/
```

Each package includes the generated firmware files, `manifest.txt`, and
`SHA256SUMS`.

### 7. Optionally Run A Manual GitHub Build

GitHub also has a manual workflow named `STM32CubeIDE Docker`.

Use it when you want a one-off hosted build for a specific target:

```text
target: rt_led or rt_led_bootloader
configuration: Debug or Release
stm32cubeide_version: 2.1.1
```

The manual workflow builds the selected target, checks firmware size, packages
artifacts, and uploads them as a workflow artifact.

### 8. Optionally Flash A Board

If hardware is available, flash the local build and validate it on the real
panel.

The bootloader is linked at:

```text
0x08000000
```

The application is linked at:

```text
0x08005000
```

When using `.bin` files, flash them at the matching address for the target.
When using an address-aware format such as `.hex`, use the addresses encoded in
the file.

Hardware validation is especially important for changes involving:

- GPIO or board pin mapping
- keyboard input behavior
- BLE or DFU behavior
- display rendering
- inverter or serial communication
- flash layout, linker scripts, EEPROM emulation, or boot handoff
- startup, clock, interrupt, or FreeRTOS timing behavior

### 9. Commit And Push

Commit the change:

```sh
git add .
git commit -m "Fix some panel behavior"
git push origin fix/some-panel-behavior
```

### 10. Open A Pull Request

Open a pull request into `main`.

The PR should describe:

- what changed
- why it changed
- how it was tested
- whether hardware was tested
- which firmware targets are affected
- whether the bootloader, application flash layout, or protocol behavior changed

GitHub Actions will run automatically on the PR:

- host tests with `./scripts/ci_test_host.sh`
- static analysis with `./scripts/ci_static_analysis.sh`
- affected firmware target detection with `./scripts/detect-firmware-targets.py`
- Dockerized STM32CubeIDE `Debug` builds for affected firmware targets
- firmware size checks with `./scripts/check-firmware-size.py`
- artifact packaging with `./scripts/package-firmware-artifacts.sh`

Firmware target detection maps changes like this:

- `rt_led/` changes build `rt_led`
- `rt_led_bootloader/` changes build `rt_led_bootloader`
- `shared/`, `desktop/`, or `tests/` changes build `rt_led`
- `.github/`, `scripts/`, CI files, and root build files build all firmware targets

The PR should not be merged until CI passes and the change is reviewed.

### 11. Merge To Main

After review and passing CI, merge the PR into `main`.

The `main` branch should always represent firmware that passes host tests,
static analysis, firmware size checks, and the STM32CubeIDE build for the
affected targets.

### 12. Create A Release Tag

After the change is merged, create a release tag from `main`:

```sh
git checkout main
git pull
git tag v1.2.3
git push origin v1.2.3
```

Use this version pattern for normal releases:

```text
vMAJOR.MINOR.PATCH
```

Examples:

```text
v1.0.0
v1.1.0
v1.1.1
```

Recommended meaning:

- `MAJOR`: incompatible protocol, flash layout, or integration behavior changes
- `MINOR`: backward-compatible feature or behavior additions
- `PATCH`: bug fixes with no expected integration changes

The release workflow also accepts tags named `Aria-*` for the Aria release
line.

### 13. GitHub Creates The Release

Pushing a tag like `v1.2.3` triggers the `Release` workflow.

GitHub Actions runs:

- host tests
- static analysis
- `Release` builds for `rt_led`
- `Release` builds for `rt_led_bootloader`
- firmware size checks for both targets
- artifact packaging for both targets

Then it creates a GitHub Release and attaches the packaged artifacts from:

```text
build/firmware/rt_led/
build/firmware/rt_led_bootloader/
```

The release artifacts include:

- `.bin` firmware images
- `.hex` images when generated by the STM32CubeIDE project
- `.elf` files for debugging
- `.map` files for symbol and memory layout debugging
- `.list` files when generated
- `manifest.txt`
- `SHA256SUMS`

### 14. Flash The Released Firmware

Download the released firmware artifacts from the GitHub Release and flash the
required target.

For application firmware, use the `rt_led` artifact and validate normal panel
behavior after flashing.

For bootloader firmware, use the `rt_led_bootloader` artifact and validate:

- bootloader startup
- handoff to the application at `0x08005000`
- BLE DFU availability
- DFU progress display
- recovery behavior when the application image is missing or invalid

After flashing any released firmware, validate the changed behavior from the PR
on real hardware before promoting the release.

## Project History

### Aug 28, 2024

- Version 1.0.6 - All programs working
- TODO: add configuration: number of intervals - HIIT program; add Gymkit behavior when connected to the watch

### Jul 08, 2024

- Version 1.0.1 - External keyboard compatibility added for all keys
- Added new programs RT G4: walk 8 mins, 200kcal burn
- Added UI/UX scope documents (rt_led/documents)
- TODO: confirm design choices on programs; add new messages on dot matrix; (extra) inverter model auto-config

### Feb 10, 2023

- Refactored NVM.

### Feb 09, 2023

-
### Feb 08, 2023

-

### Feb 07, 2023

-

### Feb 06, 2023

- Drafting BLE Interface
- Drafting Modbus Interface

### Feb 03, 2023

- See git log

### Feb 02, 2023

- See git log

### Feb 01, 2023

- Embedded NVM
- Display Bridge (Driver - Dot Matrix / Segment Displays)

### Jan 31, 2023

- NVM Configuration load/save

- Temp Calories Calculation (based on previous panel)

### Jan 30, 2023

- See git log

### Jan 27, 2023

- See git log

### Jan 26, 2023

- See git log

### Jan 25, 2023

- See git log

### Jan 24, 2023

- See git log

### Jan 23, 2023

- Shortcut keys handler

- Inverter communication thread

### Jan 17, 2023

- Fixed - Pullup on Board PD0 and PA15 (TS01 and T05)
- Refactoring Desktop Panel


### Jan 16, 2023

- Troubleshooting Pullup on Board. PD0 and PA15 (TS01 and T05)

### Jan 13, 2023

- Added current programs

### Jan 12, 2023

- Added temporary display test to hardware board.

### Jan 11, 2023

- Added hardware schematic (rt_led/documents)

### Jan 10, 2023

- Application Development

### Jan 09, 2023

- Setup and configured a new STM32 project
- Started drafting the application
