# Phantom FET Overview

## Role In The Build
`Phantom FET` is the first plugin-level vertical slice for Phantom Mix Suite. It is not just the first product in the lineup; it is the integration proof that the shared detector, gain-computer, calibration, oversampling, metering, and validation layers compose cleanly.

## Implementation Boundary
In scope for the first product slice:
- input transformer stage
- input gain
- peak detector path
- FET gain computer
- GR-dependent saturation stage
- output stage / output transformer behavior
- output trim
- gain-reduction and output metering
- oversampling mode selection

Out of scope until upstream systems are stable:
- UI art direction and final visual design
- intelligence recommendations beyond core assist hooks
- downstream plugin-family abstractions that are not required by the FET path

## Required Upstream Dependencies
`plugin/phantom-fet` depends on:
1. `core/dsp-engine`
2. `core/oversampling-engine`
3. `core/calibration-system`
4. `dsp/detector-framework`
5. `dsp/gain-computer-framework`
6. `dsp/fet-engine`
7. minimal validation-harness support for timing, transfer, and harmonic checks

The plugin branch must compose these primitives. It must not re-implement them.

## Required Product Features
- ratio modes: `4:1`, `8:1`, `12:1`, `20:1`
- all-buttons mode
- attack control
- release control
- input trim
- output trim
- meter modes
- oversampling quality selection

## Locked Behavior Requirements
- ratio modes must be measurably distinct, not simple label changes
- all-buttons mode must be a real alternate behavior, not a `20:1` alias
- distortion must increase with gain reduction, not only with input drive
- gain-reduction telemetry must feed both metering and validation tools
- calibration must keep nominal-level operation usable before heavy saturation

## Validation Gate For First Delivery
The first integrated `Phantom FET` branch is not ready for merge until it can demonstrate:
- fast but stable attack behavior across sample rates
- release behavior that stays energetic rather than lazy
- measurable GR-linked harmonic growth
- distinct all-buttons behavior relative to standard ratio modes
- deterministic state restore for ratio, timing, and meter mode behavior

## Current Blockers
- the final threshold/input control law is still listed as an open question in `implementation plan.md`
- validation snapshot format is not finalized yet

Until those are resolved, implementation may proceed on internal graph composition and measurable DSP behavior, but not on final public control semantics or golden-reference storage conventions.

## Downstream Handoff
- DSP Lead may use this overview as the contract for FET-engine integration points.
- Plugin Engineer may use it as the minimum product slice boundary.
- Validation Engineer may use it to scope the first plugin acceptance suite.
