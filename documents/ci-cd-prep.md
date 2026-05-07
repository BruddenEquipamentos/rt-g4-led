# CI/CD Preparation

This repository is being prepared around a small script contract first. CI should call scripts in `scripts/` rather than baking IDE-local paths directly into a workflow.

## Current Build Gates

- `scripts/build-desktop.sh` configures and builds the desktop harness with CMake.
- `scripts/build-firmware.sh rt_led Debug` builds the STM32 application firmware from the generated `rt_led/Debug` makefile.
- `scripts/build-cubeide.sh rt_led Debug` imports an STM32CubeIDE project into a temporary workspace and runs a headless clean build.
- `scripts/setup-local-deps.sh` prepares local build-only links, currently `rt_led_bootloader/Drivers -> ../rt_led/Drivers`.
- `scripts/detect-firmware-targets.py` maps changed paths to the affected STM32CubeIDE projects: `rt_led` and `rt_led_bootloader`.
- `scripts/ci_test_host.sh` runs the host-side build/test contract.
- `scripts/ci_static_analysis.sh` wraps `scripts/run_cppcheck.sh`. It reports cppcheck findings without failing by default; set `CPPCHECK_ERROR_EXITCODE=1` once the baseline is clean.
- `scripts/package-firmware-artifacts.sh <project> <configuration>` collects `.bin`, `.hex`, `.elf`, `.map`, `.list`, `manifest.txt`, and `SHA256SUMS` under `build/firmware/<project>/`.
- `scripts/ci.sh` initializes submodules, then runs the desktop and application firmware builds.
- `.github/workflows/ci.yml` runs changed-target PR/push CI with host tests, static analysis, and a dynamic firmware build matrix.
- `.github/workflows/firmware-cubeide.yml` is a manual single-target CubeIDE build workflow.
- `.github/workflows/release.yml` builds all firmware targets from release tags and attaches packaged artifacts to a GitHub Release.

## Verified Locally

- `scripts/build-desktop.sh`
- `scripts/build-cubeide.sh rt_led Debug`

The local STM32CubeIDE headless build produced `rt_led/Debug/rt_led.elf`, `rt_led/Debug/rt_led.bin`, `rt_led/Debug/rt_led.map`, and size output.

## Known Gaps Before Full CI

- `rt_led/Debug` is ignored by Git, but the firmware build currently depends on generated makefiles there.
- `rt_led/Drivers` and `rt_led/Middlewares` are ignored by Git, but the firmware build needs those STM32Cube dependencies.
- `rt_led_bootloader` does not currently have a committed/generated build tree equivalent to `rt_led/Debug`; local scripts create the missing `Drivers` link before build.
- `rt_led_bootloader/Drivers` depends on the application project's local STM32 driver tree. A clean checkout still needs a policy for restoring or committing `rt_led/Drivers`.
- `shared/ble/ble_protocol` is a submodule and must be intentionally pinned before CI is expected to reproduce a build.

## Recommended Next Steps

1. Decide whether STM32Cube dependencies should be committed, restored from a pinned package cache, or regenerated in CI. This remains the main clean-clone reproducibility gap.
2. Replace the IDE-generated firmware makefile dependency with a standalone CMake or checked-in Make build if clean-clone reproducibility is required.
3. Add a bootloader local build script once the bootloader has a reproducible non-CubeIDE build path.
4. Verify the hosted STM32CubeIDE action can restore or generate all required STM32 dependencies from a clean checkout.
5. Tighten cppcheck suppressions or baseline handling as the static-analysis surface becomes clean.
