# Phantom Mix Suite Implementation Plan

## Purpose
This plan translates the suite DSP architecture spec, the current repository architecture, and `.agent/MISSION.md` into an execution roadmap that is specific enough to guide branch creation, module ownership, and delivery order.

## Planning Conclusions From Current Specs
- Phantom Mix Suite is a shared DSP platform first and a plugin lineup second.
- The highest-risk work is not UI polish or host wiring; it is detector behavior, gain computer timing, calibration, oversampling, and nonlinear stage interaction.
- The current repository mission order is correct: foundation before products, products before intelligence expansion.
- The existing `specs/` tree contains the right topic areas, but the discovered plugin-level and shared-DSP markdown files are still empty and must be filled before implementation work can be validated against them.
- The first vertical slice should be `Phantom FET` because it exercises detector speed, gain computer behavior, oversampling, calibration, and gain-reduction-dependent saturation in a focused graph.

## Delivery Doctrine
- Build measurable behavior before flavor extras.
- Treat calibration and operating level as core DSP features.
- Keep shared DSP primitives framework-agnostic in `dsp/`.
- Keep plugin graph composition in `plugins/`.
- Add intelligence only where signal measurements can justify recommendations.
- Do not begin broad plugin implementation until shared validation tooling exists.

## Milestone Roadmap

### Milestone 0: Spec Consolidation
Goal: convert the architecture blueprint into executable module specs.

Deliverables:
- Complete `specs/shared-dsp/detector-framework.md`.
- Complete `specs/plugin-fet/fet-gain-computer.md`.
- Complete `specs/plugin-console/ssl-routing-behavior.md`.
- Add companion specs for calibration, oversampling, nonlinear families, and validation harness if not already present.
- Define plugin acceptance criteria for `Phantom Console`, `Phantom FET`, and `Phantom Tube EQ`.

Exit criteria:
- Each foundational DSP module has inputs, outputs, equations, edge cases, and test expectations documented.
- Each Phase 1 plugin has a signal path spec, control map spec, and validation targets.

### Milestone 1: Shared DSP Infrastructure
Goal: create the core platform required by every plugin.

Deliverables:
- Shared audio types and channel layout abstractions.
- Parameter smoothing classes for continuous and stepped controls.
- Calibration system with selectable operating references.
- Oversampling manager with per-module configuration and latency reporting.
- Metering primitives for input, output, GR, harmonic activity estimate, and saturation proximity.
- Shared parameter/state serialization contract.

Exit criteria:
- Core systems compile independently of plugin-specific graphs.
- Latency and calibration behavior are testable.
- No-denormal protections and sample-rate support are in place.

### Milestone 2: Core Modeling Engines
Goal: implement the DSP primitives that define plugin character.

Deliverables:
- Detector family implementations: RMS, peak/rectified, feedback topology hooks, program-dependent opto scaffolding.
- Gain computer framework with hard knee, soft knee, variable knee, attack/release envelope application, and auto makeup hooks.
- Nonlinear models: transformer, tube, VCA, FET, diode/zener, and Vari-Mu scaffolding.
- Noise, crosstalk, component variance, and optional power-supply behavior interfaces.
- Filter framework: active IIR blocks plus passive/WDF-ready abstractions.

Exit criteria:
- Detector and gain computer timing is measurable against spec.
- Nonlinear stages are calibration-aware and oversampling-aware.
- Shared primitives can be composed without plugin code duplication.

### Milestone 3: Validation Harness
Goal: prevent subjective-only development.

Deliverables:
- Test harness for sweeps, stepped sine, THD+N, IMD, transfer curves, burst tests, amplitude steps, and memory-state tests.
- Golden-reference baseline format for frequency response, timing behavior, harmonic growth, CPU, and oversampling correctness.
- Deterministic seed validation for component variance.
- Benchmark entry points for CPU and high-sample-rate stress tests.

Exit criteria:
- Foundational DSP modules have regression coverage.
- Measurement scripts can be run per module and per plugin.

### Milestone 4: Phase 1 Plugins
Goal: ship the first usable product set on top of the platform.

Build order:
1. `Phantom FET`
2. `Phantom Console`
3. `Phantom Tube EQ`

Rationale:
- `Phantom FET` is the best first vertical slice for validating detector speed, GR-linked distortion, oversampling, and calibration.
- `Phantom Console` should follow once routing and shared dynamics blocks are stable.
- `Phantom Tube EQ` should follow once passive interaction and transformer/tube makeup stages are reliable.

### Milestone 5: Intelligence Core v1
Goal: add advisory analysis without destabilizing the audio path.

Deliverables:
- Shared feature extraction: RMS, peak, crest factor, transient density, spectral tilt, low-mid accumulation, GR amount, saturation proximity.
- Rules engine for warnings and recommended starting points.
- Plugin-specific hint adapters for Console, FET, and Tube EQ.
- UI contract for Analyze, Assist, Health meter, and hint presentation.

Exit criteria:
- Recommendations are tied to measurable thresholds.
- Intelligence can be disabled without changing sound.

## Branch-by-Branch Execution Plan

### Foundation Branches

#### `core/dsp-engine`
Scope:
- Base processing interfaces, sample-rate lifecycle, channel configuration, shared state containers, metering buses.

Modules:
- `core/audio/`
- `core/processing/`
- `core/state/`
- `core/metering/`

Definition of done:
- Plugin graphs can depend on this layer without importing plugin-specific code.

#### `core/oversampling-engine`
Scope:
- Polyphase up/down sampling, latency accounting, quality tiers, per-block oversampling policy.

Modules:
- `core/oversampling/`
- `tests/oversampling/`

Definition of done:
- Shared API supports native, 2x, 4x, and future 8x paths.

#### `core/calibration-system`
Scope:
- Operating level references, headroom mapping, nonlinear onset definitions, calibration-aware metering.

Modules:
- `core/calibration/`
- `data/calibration/`

Definition of done:
- Plugins can query nominal level, soft saturation onset, and clipping threshold from a common interface.

### DSP Branches

#### `dsp/detector-framework`
Scope:
- RMS, peak, rectified, feedback, and sidechain pre-filter support.

Dependencies:
- `core/dsp-engine`

Tests:
- Attack/release timing tests
- sample-rate consistency tests
- detector filter response tests

#### `dsp/gain-computer-framework`
Scope:
- Compression curves, knees, envelope application, ratio modes, makeup gain support.

Dependencies:
- `dsp/detector-framework`
- `core/calibration-system`

Tests:
- static curve matching
- gain reduction envelope correctness
- all-buttons and alternate topology behavior hooks

#### `dsp/transformer-model`
Scope:
- Practical production transformer stage with contouring, memory term, calibration-sensitive onset.

Tests:
- transfer curve capture
- harmonic growth vs level
- LF saturation behavior

#### `dsp/fet-engine`
Scope:
- FET audio path nonlinearity with explicit gain-reduction dependency.

Dependencies:
- `dsp/detector-framework`
- `dsp/gain-computer-framework`
- `core/oversampling-engine`

Tests:
- GR-linked distortion increase
- transient retention vs attack settings
- all-buttons mode behavior shift

#### `dsp/wdf-filter-framework`
Scope:
- Passive-network abstractions for Pultec and future Massive Passive work.

Tests:
- parameter interaction stability
- curve-fit comparison vs target responses

### Plugin Branches

#### `plugin/phantom-fet`
Scope:
- First end-to-end product using input transformer, detector, FET GR, GR-dependent saturation, output stage, meters, and core assist hooks.

Required subfeatures:
- ratio modes
- all-buttons mode
- meter modes
- input/output trim
- oversampling quality selection

Validation targets:
- extreme speed without instability
- aggression and forwardness under GR
- distortion rising with gain reduction, not only input drive

#### `plugin/phantom-console`
Scope:
- Channel strip graph with filters, dynamics, EQ, routing matrix, sidechain options, and optional analog layers.

Required subfeatures:
- 02 and 242 EQ variants
- compressor soft/hard knee
- log/linear release
- dynamics pre/post EQ routing
- gate hysteresis
- filters/EQ to sidechain

Validation targets:
- routing materially changes compression tone
- clean baseline with punchy dynamics behavior

#### `plugin/phantom-tube-eq`
Scope:
- Passive EQ graph with LF/HF interaction rules, tube makeup gain, and transformer output behavior.

Required subfeatures:
- passive insertion loss compensation
- broad musical curves
- low-end contour interaction
- HF silk without brittle edge

Validation targets:
- passive interaction is audible and measurable
- makeup stage sweetening remains subtle at nominal level

### Intelligence Branches

#### `ai/audio-analysis-engine`
Scope:
- feature extraction and measurement normalization shared across plugins.

Outputs:
- crest factor
- transient density
- spectral tilt
- low-mid accumulation
- saturation proximity

#### `ai/mix-assistant`
Scope:
- rules engine and plugin-specific hint generation.

Rules to prioritize first:
- FET transient flattening warning
- Console sidechain HPF suggestion
- Tube EQ mud-risk and harshness guidance

## Suggested Module Layout

### `core/`
- processing lifecycle and shared abstractions
- calibration interfaces
- oversampling service
- state and preset support
- metering and latency reporting

### `dsp/`
- `detectors/`
- `gain_computers/`
- `nonlinear/`
- `filters/`
- `analog_realism/`

### `plugins/`
- `phantom_fet/`
- `phantom_console/`
- `phantom_tube_eq/`
- each plugin owns graph assembly, parameter mapping, and plugin-facing state

### `tests/`
- `unit/`
- `integration/`
- `regression/`
- `measurement/`

## Immediate Next Actions
1. Write the missing foundation specs in `specs/shared-dsp/` and `specs/plugin-*`.
2. Start `core/dsp-engine`, `core/oversampling-engine`, and `core/calibration-system` in parallel where possible.
3. Implement `dsp/detector-framework` before any plugin-specific dynamic processor work.
4. Use `plugin/phantom-fet` as the first integrated delivery target.
5. Build the validation harness before broadening into additional Phase 1 plugins.

## Developer Handoff Package

### What Must Exist Before Coding Starts
- A non-empty spec for every foundational DSP subsystem touched by Milestone 1 or 2.
- Branch ownership and dependency order agreed in writing.
- Validation expectations attached to each DSP module, not deferred until after implementation.
- Shared terminology for level domains, calibration references, detector outputs, and graph order.

### Minimum Ready-To-Code Document Set
- `implementation plan.md`
- `.agent/MISSION.md`
- `.agent/ARCHITECTURE.md`
- `specs/shared-dsp/detector-framework.md`
- `specs/shared-dsp/calibration-system.md`
- `specs/shared-dsp/oversampling-framework.md`
- `specs/shared-dsp/nonlinear-model-families.md`
- `specs/shared-dsp/validation-harness.md`
- `specs/plugin-fet/fet-gain-computer.md`
- `specs/plugin-fet/acceptance.md`
- `specs/plugin-console/ssl-routing-behavior.md`
- `specs/plugin-console/acceptance.md`
- `specs/plugin-tube-eq/acceptance.md`


### Handoff Rules For Engineers
- Shared primitives go in `dsp/` and must not depend on plugin product code.
- Plugin branches may compose primitives but must not fork detector or gain-computer logic unless the spec is updated first.
- Every new DSP block must ship with at least one measurable validation strategy.
- Calibration assumptions must be explicit in code and tests.
- If implementation behavior and spec diverge, update the spec before merging or correct the implementation.

## Actionable Workstreams By Branch

### Workstream: `core/dsp-engine`
- define common processing interfaces and processor lifecycle
- define shared channel-mode types and buffer conventions
- define common meter buses and measurement taps
- define reset, prepare, and block-processing contracts used by all DSP modules

### Workstream: `core/oversampling-engine`
- define oversampling mode enum and quality policies
- define latency reporting contract to plugin layer
- choose filter topology and write benchmark acceptance thresholds
- define bypass/native-rate behavior for modules that do not require oversampling

### Workstream: `core/calibration-system`
- define supported operating references and conversion helpers
- define per-plugin nominal level, soft saturation onset, and clipping threshold representation
- define calibration metadata API shared by nonlinear stages and meters

### Workstream: `dsp/detector-framework`
- implement coefficient utilities first
- implement peak and RMS detectors second
- add stereo linking and sidechain filtering third
- expose debug taps for validation harness fourth

### Workstream: `dsp/gain-computer-framework`
- implement dB-domain curve evaluation
- implement attack/release application against target GR
- define ratio-mode hook points for plugin-specific behaviors
- define makeup-gain extension points without hard-coding product policy

### Workstream: `dsp/transformer-model`
- start with production-safe level-1 model
- keep memory term optional and testable
- define calibration-aware onset points from shared system
- validate LF saturation and harmonic trend before adding extra realism layers

### Workstream: `dsp/fet-engine`
- accept detector/gain-computer outputs as explicit inputs
- expose normalized GR signal to the nonlinear stage
- define all-buttons behavior as a full alternate mode, not a ratio alias
- verify oversampling integration early

### Workstream: `plugin/phantom-fet`
- wire the smallest viable end-to-end graph first
- bring up ratio modes and meter modes before secondary polish
- connect GR telemetry to both metering and validation tools
- document tuning notes as behavior decisions are made

### Workstream: `plugin/phantom-console`
- finalize routing graph contract before EQ coefficient tuning
- keep section-order switching explicit in code
- validate gate and compressor sidechain sources independently
- avoid hidden UI-only routing states

### Workstream: `plugin/phantom-tube-eq`
- finalize passive interaction model before cosmetic controls
- define insertion loss compensation policy explicitly
- validate broad-curve behavior and HF smoothness at nominal and driven levels

### Workstream: `ai/audio-analysis-engine`
- define analysis window sizes and update cadence
- normalize measurement outputs for cross-plugin use
- keep outputs advisory-only and decoupled from the audio path

## Pre-Coding Open Questions To Resolve In Specs
- exact oversampling quality tiers and CPU budget targets
- whether detector stereo linking defaults to max, energy-sum, or plugin-specific policy
- final threshold/input control law for Phantom FET user interaction
- exact routing permutations exposed in Phantom Console UI vs internal-only support
- validation data format for golden references and measurement snapshots

## Recommended Sequence For Developer Onboarding
1. Read `implementation plan.md` for branch order and system boundaries.
2. Read `.agent/ARCHITECTURE.md` for ownership and folder discipline.
3. Read the relevant `specs/` file for the branch being implemented.
4. Confirm required tests before writing DSP code.
5. Implement the smallest measurable slice first, then widen behavior.

## Risks To Control Early
- Empty or vague specs causing branch drift.
- Oversampling added too late, forcing nonlinear rewrites.
- Subjective tuning without regression baselines.
- Plugin branches re-implementing shared DSP behavior.
- Intelligence rules being added before measurement features are trustworthy.

## Success Criteria For The First Execution Cycle
- Shared core can host at least one full plugin graph.
- Detector, gain computer, calibration, and oversampling systems are reusable.
- `Phantom FET` produces measurable GR-linked nonlinear behavior.
- Validation harness catches timing, aliasing, and transfer-curve regressions.
- The repo has a clear branch plan and module ownership path before full-scale coding begins.
