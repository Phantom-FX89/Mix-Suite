# Shared DSP Spec: Validation Harness

## Purpose
Define the measurement and regression framework used to verify Phantom Mix Suite DSP behavior objectively before and after plugin integration.

## Design Goals
- Prevent purely subjective tuning from driving core DSP decisions.
- Make detector, gain computer, filter, and nonlinear behavior measurable and repeatable.
- Support both module-level and plugin-level validation.
- Capture regressions in timing, transfer curves, aliasing, CPU cost, and state behavior.

## Scope
This spec covers:
- test categories
- fixture and baseline formats
- deterministic execution rules
- regression workflow expectations
- module and plugin acceptance hooks

## Core Principles
- every foundational DSP module must have at least one measurable validation path
- golden references must represent expected behavior, not accidental implementation quirks
- deterministic seeds are mandatory for any variance-enabled test path
- perceptual listening complements measurement, but does not replace it

## Required Test Categories

### Linear Response
- logarithmic sweeps
- fixed-frequency stepped sine tests
- phase and group delay estimation
- EQ curve matching

Targets:
- filters
- passive network approximations
- sidechain filter paths where applicable

### Static Nonlinearity
- THD+N vs level
- harmonic spectrum at multiple drive points
- transfer-curve capture

Targets:
- transformer model
- tube model
- FET path
- diode/zener path

### Intermodulation
- SMPTE two-tone
- CCIF two-tone
- multi-tone density tests

Targets:
- nonlinear stages
- full plugin chains under driven conditions

### Dynamic Behavior
- burst tests for attack/release
- amplitude step tests
- crest-factor response tests
- program-dependent release verification

Targets:
- detectors
- gain computers
- full compressors

### State and Memory Tests
- opto exposure history tests
- PSU sag recovery tests
- component variance determinism tests
- preset restore and reset consistency tests

## Harness Architecture Requirements

### Test Inputs
Harness must support generating or loading:
- sine tones
- sweeps
- impulses
- bursts
- multi-tone signals
- representative audio fixtures for perceptual smoke tests

### Execution Model
- module-level tests should run without plugin shell dependencies where practical
- plugin-level tests should run through the actual composed signal graph
- all tests must record sample rate, block size, calibration mode, oversampling mode, and seed where relevant

### Output Artifacts
Harness should be able to emit:
- scalar assertions for CI
- machine-readable measurement data
- plots or plot-ready exports for manual review
- baseline snapshots for regression comparison

## Golden Reference Rules
- references must be versioned with clear ownership
- references must include metadata describing sample rate, calibration mode, oversampling mode, and test signal
- reference updates require justification tied to a spec or intentional behavior change
- developers must not overwrite baselines casually after a failing test

## Determinism Rules
- variance engine tests must use explicit seeds
- random-noise-assisted tests must log seed and generator settings
- reset-state tests must prove repeatable results from identical initial conditions

## Recommended Directory Ownership
- `tests/unit/` for small deterministic module tests
- `tests/integration/` for graph and module-composition tests
- `tests/regression/` for golden-reference checks
- `tests/measurement/` for analyzers, fixtures, and result exporters
- `benchmarks/` for CPU/performance validation

## Acceptance Hooks By Module Family

### Detector Framework
- attack/release timing within tolerance
- sample-rate consistency
- sidechain filter influence measurable

### Gain Computer Framework
- static curve correctness
- envelope application correctness
- ratio/all-buttons mode distinction where relevant

### Oversampling Framework
- latency reporting correctness
- alias reduction under nonlinear load
- stable reset and mode switching

### Calibration System
- onset thresholds move predictably with calibration reference
- saturation proximity reporting remains coherent

### Plugin Graphs
- signal-path order and routing behavior
- metering correctness
- preset/state determinism

## Perceptual Validation Workflow
- always compare dry vs processed at matched loudness where practical
- use multiple source classes, not only test tones
- include fast mix-context checks for plugin usability
- log subjective observations separately from measurement pass/fail criteria

## CI and Automation Requirements
- fast deterministic tests should run in CI on every relevant branch
- heavier measurement suites may run on scheduled or gated workflows if execution time is high
- benchmark thresholds should flag regressions rather than silently drift

## Definition of Done
- Shared harness can validate linear, nonlinear, dynamic, and stateful behaviors.
- Golden-reference storage and update rules are documented.
- Module and plugin teams have a common place to add measurable acceptance coverage.
- Regression workflow exists before broad plugin tuning begins.
