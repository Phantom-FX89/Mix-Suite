# Shared DSP Spec: Calibration System

## Purpose
Define the shared calibration framework used across Phantom Mix Suite so that detector thresholds, saturation onset, metering, and plugin gain staging all operate from the same reference model.

## Design Goals
- Make level-dependent DSP behavior consistent across plugins.
- Support authentic analog operating references without locking users to a single workflow.
- Provide a single source of truth for nominal level, nonlinear onset, and clipping thresholds.
- Ensure calibration state is visible to DSP, metering, presets, and intelligence layers.

## Supported Reference Modes
Required global references:
- `0 VU = -18 dBFS`
- `0 VU = -20 dBFS`
- `0 VU = -14 dBFS`

Rules:
- one of these modes must be active for every plugin instance
- plugin presets must preserve the selected calibration context or explicitly declare whether they inherit the global suite setting
- calibration changes must not invalidate plugin state or produce unstable nonlinear behavior

## Core Concepts

### Nominal Operating Level
Defines the expected level where the plugin behaves closest to its intended clean baseline.

### Nonlinear Onset
Defines the level where harmonic growth begins to become measurable.

### Soft Saturation Onset
Defines the level where soft clipping or headroom compression becomes musically apparent.

### Hard Clipping Threshold
Defines the internal maximum safe operating region before hard limiting or clip protection behavior occurs.

Each plugin shall provide these values relative to the selected calibration reference.

## Required Public Interface

### Global Calibration State
Shared system shall expose:
- active reference mode
- reference offset in dBFS terms
- helpers for converting between dBFS, nominal operating level, and normalized drive regions

### Per-Plugin Calibration Profile
Each plugin profile shall expose at minimum:
- `nominalLevelDbfs`
- `nonlinearOnsetDbfs`
- `softSatOnsetDbfs`
- `clipThresholdDbfs`
- optional clean-headroom margin

### DSP-Facing Queries
Shared API should support queries such as:
- is signal below nominal, near nominal, driven, or clipping
- normalized drive amount above nominal
- normalized saturation proximity

## Behavioral Rules

### Detector and Dynamics Interaction
- detector framework must not hard-code calibration thresholds
- gain computers and plugin graphs may use calibration metadata to shape threshold defaults and assist recommendations
- compressor behavior at nominal level must remain musically useful rather than immediately overdriven

### Nonlinear Stage Interaction
- transformer, tube, FET, and other nonlinear models must read calibration metadata rather than invent plugin-local drive assumptions
- changing calibration reference must change effective drive/headroom relationship in a controlled and testable way
- calibration changes must preserve relative behavior trends even if absolute onset point shifts

### Metering Interaction
- meters should be able to show nominal reference alignment
- saturation proximity and process-health meters must derive from calibration-aware thresholds
- clip indication must reflect the plugin's calibrated internal headroom model, not only host output clipping

## Parameter and State Requirements
- calibration mode must be persistable in host state or clearly inherited from suite-global state
- automation of calibration mode is not required for first implementation and may be disallowed if it risks audible discontinuity
- any per-plugin calibration override must be explicit and visible in state serialization

## Intelligence Layer Hooks
Calibration system must expose data required for:
- gain staging advice
- saturation proximity warnings
- process-health scoring
- inter-plugin chain headroom analysis

## Recommended Data Model
Each plugin should register a calibration profile such as:

```text
PluginCalibrationProfile {
    pluginId
    nominalOffsetFromReferenceDb
    nonlinearOnsetOffsetDb
    softSatOffsetDb
    clipOffsetDb
}
```

Implementation may use any equivalent structure, but the semantic fields must remain available.

## Validation Requirements

### Unit Tests
- verify dBFS conversion for all supported reference modes
- verify normalized drive/saturation proximity mapping
- verify plugin profile lookup and deterministic defaults

### Integration Tests
- verify calibration change shifts nonlinear onset in expected direction
- verify metering thresholds track calibration state
- verify preset save/load restores calibration behavior correctly

### Perceptual Checks
- identical source material should feel cleaner or more driven as calibration reference changes, without changing the core plugin identity

## Definition of Done
- Shared calibration API exists and is reusable by all plugins.
- Each plugin can declare nominal, nonlinear, soft-sat, and clipping regions.
- Nonlinear and metering systems can query calibration state consistently.
- Validation coverage proves calibration changes alter headroom behavior predictably.
