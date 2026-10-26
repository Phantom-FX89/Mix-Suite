# Phantom Mix Suite

Shared C++20 DSP library and directory scaffold for a mixing suite that is not built yet. The CMake project name is `PhantomMixSuiteDSP`. This checkout has library sources under `core/` and `dsp/`, empty plugin slots, and no license text.

## Status

| Item | State in this tree |
| --- | --- |
| DSP sources | Present. Eight translation units under `core/` and `dsp/`. |
| Static library `phantom_dsp` | Declared in `CMakeLists.txt`. The CMake generate step fails before it is built. |
| Tests | `tests/TestMain.cpp` and `tests/TestRunner.h` only. The six test sources CMake lists are missing, so `ctest` never runs. |
| Plugins, UI, presets, hosts | Directories with `.gitkeep` files. |
| JUCE and other third-party SDKs | Empty placeholder directories. No `.gitmodules`. |
| CI | No `.github` workflows. |
| License | `LICENSE` is a zero-byte file. It states no terms. |
| Changelog | `CHANGELOG.md` is empty. |

## What is implemented

`CMakeLists.txt` defines one static library, `phantom_dsp`, from:

- `core/calibration/Calibration.cpp`
- `core/metering/Metering.cpp`
- `core/oversampling/Oversampling.cpp`
- `dsp/analog/AnalogPrimitives.cpp`
- `dsp/detectors/Detectors.cpp`
- `dsp/filters/Biquad.cpp`
- `dsp/gain/GainComputer.cpp`
- `dsp/nonlinear/Nonlinear.cpp`

Public includes use the repository root (`target_include_directories` is `${CMAKE_CURRENT_SOURCE_DIR}`).

| Code | What the sources do |
| --- | --- |
| `core/processing/Types.h` | Channel modes, dB helpers, and `OversamplingMode` (`x1`, `x2`, `x4`, `x8HQ`). |
| `core/processing/ParameterSmoother.h` | Header-only smoother. Included directly. It is not one of the library `.cpp` files. |
| `core/audio/AudioBlock.h` | `AudioBlockView` and `OwnedAudioBlock`. |
| `core/calibration/` | Reference modes for 0 VU at -18, -20, or -14 dBFS, plugin profile registration, and drive-region classification. |
| `core/metering/` | Level meter and gain-reduction meter with attack/release ballistics. |
| `core/oversampling/` | 1x, 2x, and 4x processing with latency queries. `x8HQ` throws `std::invalid_argument` with the message `x8HQ is reserved for a later quality tier`. |
| `dsp/filters/` | One direct-form biquad, plus coefficient helpers for high-pass, bell, low shelf, and high shelf. |
| `dsp/detectors/` | Peak and RMS detectors, optional stereo link policies, sidechain high-pass / tilt / bell, and `ProgramDependentDetectorScaffold`. |
| `dsp/gain/` | One dB-domain gain computer: hard, soft, or variable knee, attack/release, and an optional auto-makeup callback. |
| `dsp/nonlinear/` | `TransformerStage`, `TubeStage`, `VcaStage`, `FetStage`, `DiodeZenerStage`, and `VariMuStage`. Each is a calibration-aware per-sample waveshaper. |
| `dsp/analog/` | Seeded component variance, white noise, a constant stereo crosstalk mix, and a power-supply memory one-pole. |

`docs/architecture/shared-dsp-foundation.md` describes this set and the limits that still apply: nonlinear families are scaffolds, oversampling quality tiers are open, and the Phantom FET threshold/input law lives outside this library.

The C++ sources have no parametric EQ class, no multi-band dynamics processor, and no `juce::AudioProcessor` type.

On a machine with g++ 13, those eight library `.cpp` files compile with:

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -I. -c <file>
```

That command is a check of the sources. It is not a CMake target, and it does not run tests.

## What is only a slot

`plugins/` names eight products. Under each product, `include/`, `source/`, `tests/`, `presets/`, and `resources/` contain only `.gitkeep`:

- `plugins/phantom-bus`
- `plugins/phantom-console`
- `plugins/phantom-fet`
- `plugins/phantom-opto`
- `plugins/phantom-passive`
- `plugins/phantom-tg`
- `plugins/phantom-tube-eq`
- `plugins/phantom-varimu`

The same is true of the rest of the product tree: `ui/`, `presets/`, `assets/`, `intelligence/`, `src/`, `config/`, `public/`, `specs/`, `tools/`, `scripts/`, `ci/`, `standalone/phantom-suite-host`, `standalone/dsp-lab`, `standalone/preset-auditioner`, `cmake/presets`, `cmake/modules`, `cmake/toolchains`, `builds/local`, `builds/ci`, and `builds/release`.

`juce/shared`, `juce/app-config`, and `juce/wrappers` are empty. `third_party/` is empty `catch2`, `clap`, `eigen`, `fmt`, `vst3sdk`, and `misc` directories. CMake never adds JUCE, the VST3 SDK, or CLAP. AU, VST3, AAX, and CLAP targets are not declared.

`docs/plugin-overviews/phantom-fet.md` is a brief for a future FET slice (ratio modes, attack, release, metering, oversampling). `plugins/phantom-fet/source/` has no sources. The brief's own blockers are still open: the threshold/input control law, and a validation snapshot format. It points at `implementation plan.md`, and that file is not in this tree.

## Prerequisites

- CMake 3.22 or newer (`cmake_minimum_required` in `CMakeLists.txt`).
- A C++20 compiler. The project sets `CMAKE_CXX_STANDARD` to 20, requires it, and turns extensions off. For Clang, AppleClang, and GNU, the targets also use `-Wall -Wextra -Wpedantic -Werror`.

No `package.json`, Conan, or vcpkg manifest is checked in. Configure does not download dependencies. JUCE is not required for this CMake file, and adding a JUCE copy elsewhere does not create plugin targets.

## Install

There is no install step. `CMakeLists.txt` has no `install()` rule. `scripts/setup` and `scripts/release` contain only `.gitkeep`. Nothing in this repository installs a plugin, a host, or a packaged library.

## Configuration

No host runs, so there is no app configuration and no secrets to fill in.

- `config/` is empty.
- No `CMakePresets.json` is shipped. `cmake/presets/` is a placeholder.
- `.gitignore` ignores `build/` (a CMake build directory) plus a few compiler and Visual Studio artifacts. The tracked `builds/` directory is placeholders, not build output.

Do not commit credentials. This tree has no env example to copy.

## Build

`docs/architecture/shared-dsp-foundation.md` asks for this sequence. It does not succeed here.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

CMake configures, then generate fails:

```text
CMake Error at CMakeLists.txt:30 (add_executable):
  Cannot find source file:

    tests/unit/AnalogPrimitivesTests.cpp
```

`add_executable(phantom_dsp_tests ...)` lists six files that are not in the repository:

- `tests/unit/AnalogPrimitivesTests.cpp`
- `tests/unit/CalibrationTests.cpp`
- `tests/unit/DetectorTests.cpp`
- `tests/unit/GainComputerTests.cpp`
- `tests/unit/NonlinearTests.cpp`
- `tests/unit/OversamplingTests.cpp`

`tests/TestMain.cpp` calls `runAnalogPrimitivesTests`, `runCalibrationTests`, `runDetectorTests`, `runGainComputerTests`, `runNonlinearTests`, and `runOversamplingTests` in namespace `phantom::testing`. Those functions are not defined in the tree. `tests/TestRunner.h` provides `PHANTOM_EXPECT_TRUE` and `PHANTOM_EXPECT_NEAR`.

Generate stops before writing a build system, so `cmake --build build` has nothing to build and `ctest` is not reached. The static library is not emitted either.

Reproduced with CMake 3.28.3 and g++ 13: generate stops on `tests/unit/AnalogPrimitivesTests.cpp`. The failure is the missing test sources. It is not a missing JUCE submodule.

## Usage

No executable, plugin binary, or command-line tool is produced.

The intended library surface is the headers in `core/` and `dsp/`, linked as `phantom_dsp`, once a build exists. `standalone/` does not contain a program that links it. Until `CMakeLists.txt` and `tests/unit/` agree on which test files exist, the documented CMake commands do not produce that library.

## Architecture

Read the docs that are actually in the tree:

1. `docs/architecture/shared-dsp-foundation.md` — shared blocks that exist, the approximations they still use, and the validation that has not been run. This is the architecture note for the code above.
2. `docs/architecture/phantom-suite-system.md` — milestone order and ownership boundaries for work that is still ahead. It references `implementation plan.md` and `specs/`. The plan file is absent. `specs/` is empty.
3. `docs/plugin-overviews/phantom-fet.md` — product brief for the first plugin slice. The plugin sources are absent.

Schemas next to the DSP types:

- `data/calibration/profile-schema.json`
- `data/detectors/debug-tap-schema.json`
- `data/harmonics/telemetry-schema.json`

Notes, separate from measured results:

- `research/transformer/hysteresis-notes.md` — pre-production modeling note. It says this repository has no transformer measurements and no hardware captures.
- `benchmarks/dsp/transformer-hysteresis-benchmark-plan.md` — a plan for a later measurement pass. `benchmarks/` has no harness code.

These paths exist and are empty:

- `docs/validation/measurement-protocol.md`
- `research/fet/1176-envelope-study.ipynb`
- `research/wdf/pultec-network-prototype.cpp`
- `CHANGELOG.md`

`.agent/` is internal agent workflow notes (`.agent/MISSION.md`, `.agent/ARCHITECTURE.md`, `.agent/ACCEPTANCE.md`, `.agent/NOTES.md`, and `.agent/workflows/`). It is not the DSP design. `.agent/RUNBOOK.md` is empty.

## Testing

The project declares a test and cannot run it.

`CMakeLists.txt` calls `enable_testing()` and `add_test(NAME phantom_dsp_tests COMMAND phantom_dsp_tests)`. Generation fails on the missing sources in [Build](#build), so there is no `ctest` result and no passing suite to report.

Placeholder test directories (`.gitkeep` only):

- `tests/fixtures/`
- `tests/integration/`
- `tests/plugin-validation/au-vst3/`
- `tests/regression/console-eq-curves/`
- `tests/unit/detectors/`
- `tests/unit/nonlinear/`

No continuous-integration workflow is configured.

## Scripts

| Path | Contents |
| --- | --- |
| `scripts/setup` | `.gitkeep` |
| `scripts/test` | `.gitkeep` |
| `scripts/format` | `.gitkeep` |
| `scripts/package` | `.gitkeep` |
| `scripts/release` | `.gitkeep` |

No setup, format, package, or release command ships with the repo.

## Contributing

No `CONTRIBUTING` file, issue template, or CI check is in the repository.

When DSP code changes, update `docs/architecture/shared-dsp-foundation.md` so it still matches `core/` and `dsp/`. Plugin-format support, UI, and license claims need the corresponding files in the tree.

Leave `LICENSE` for the owner. This README does not fill it in.

## License

`LICENSE` is a zero-byte file. It names no copyright holder and no license terms. The tree does not grant a license. An earlier README described this project as MIT; that text did not match `LICENSE`, and this file does not repeat it. Treat the code as unlicensed until the owner commits a real license.
