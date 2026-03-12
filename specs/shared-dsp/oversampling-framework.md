# Shared DSP Spec: Oversampling Framework

## Purpose
Define the oversampling infrastructure used by nonlinear and feedback-sensitive Phantom Mix Suite modules.

## Design Goals
- Reduce objectionable aliasing without imposing unnecessary CPU cost.
- Apply oversampling per module, not as a blanket always-on global process.
- Make oversampling state explicit, measurable, and latency-reportable.
- Keep integration simple for DSP modules that require native, 2x, 4x, or future 8x operation.

## Supported Modes
Initial required modes:
- `1x` native
- `2x`
- `4x`

Future-ready mode:
- `8x HQ`

## Default Policy Guidance
- linear filters only: `1x` or `2x`
- modest transformer or tube coloration: `2x` or `4x`
- FET and hard nonlinear compression stages: `4x`
- vari-mu or complex nonlinear tube stages: `4x` or `8x HQ`
- passive network stages without strong nonlinearity: native unless paired with nonlinear stage

These are policy defaults; plugin graphs must still declare their chosen mode per stage.

## Scope
This spec covers:
- oversampling ratios
- filter requirements
- latency reporting
- bypass and state behavior
- plugin integration contract
- validation requirements

## Required Public Interface

### Lifecycle
Each oversampling wrapper shall support:
- `prepare(sampleRate, maxBlockSize, numChannels)`
- `reset()`
- `setMode(mode)`
- `getLatencySamples()`
- `processBlock(input, processor)` where `processor` operates at the upsampled rate

### Processor Contract
The oversampling system must allow wrapped processors to know:
- current effective sample rate
- current oversampling factor
- block size at oversampled domain

## Filter Requirements
- use polyphase up/down sampling filters
- alias rejection must be suitable for aggressive nonlinear processing
- passband ripple and phase behavior must be documented for the chosen implementation
- transition design should balance CPU and quality, with at least one production-quality mode available for shipping

## Latency Requirements
- added latency must be queryable per module
- plugin graph must be able to sum module latencies into host-reported latency
- changing oversampling mode must update latency reporting correctly
- first implementation may restrict mode changes during transport if needed for safe host behavior, but state restore must remain correct

## Routing and Graph Rules
- oversampling must be attachable per module or per graph section
- plugin graph decides where up/down boundaries occur
- multiple adjacent nonlinear stages may share one oversampled island if this reduces redundant conversions and preserves intended interaction
- oversampling boundaries must be explicit in the graph design

## Bypass and Fallback Rules
- modules that do not need oversampling must be able to remain at native rate
- if CPU-saving or draft mode exists later, plugin behavior must still remain stable even if a lower oversampling tier is selected
- bypassing an oversampled stage should avoid invalid state or stale latency reporting

## Numerical and Performance Requirements
- all modes must be stable at 44.1 kHz through 192 kHz host rates
- internal processing should support double precision where nonlinear blocks demand it
- repeated mode switching must not leak memory or leave stale buffers
- framework must be benchmarkable independently of plugin shell code

## Recommended Data Model
Suggested concepts:
- `OversamplingMode`
- `OversamplingQuality`
- `OversampledProcessorContext`
- `OversamplingLatencyInfo`

Implementation details may vary, but these responsibilities must exist.

## Integration Guidance By Module Family

### FET Compression Path
- detector may remain native or use dedicated policy if proven necessary
- GR-linked nonlinear audio path should use `4x` by default

### Transformer and Tube Stages
- modest coloration may use `2x`
- driven stages should support `4x`

### Passive EQ Stages
- native rate preferred unless paired directly with nonlinear makeup stage

## Validation Requirements

### Unit Tests
- mode selection and effective sample-rate reporting
- latency query correctness per mode
- reset determinism

### Integration Tests
- verify wrapped processor receives correct oversampled sample rate
- verify graph latency updates when mode changes
- verify adjacent oversampled blocks do not produce invalid buffer sizing or channel mismatch

### Measurement Tests
- alias rejection comparison between `1x`, `2x`, and `4x` for representative nonlinear stages
- null/near-null sanity checks for linear-path processing when oversampling is engaged but no nonlinearity is active
- CPU benchmark capture per mode

## Definition of Done
- Shared oversampling wrapper supports `1x`, `2x`, and `4x` modes.
- Latency is reportable and graph-integratable.
- Nonlinear modules can run inside an oversampled processing island.
- Validation proves reduced aliasing and stable operation across sample rates.
