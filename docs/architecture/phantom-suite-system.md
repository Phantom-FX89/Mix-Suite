# Phantom Mix Suite System Architecture

## Purpose
This document extracts the active implementation boundary from `implementation plan.md` and the current shared/plugin specs. It exists to keep branch sequencing, scope boundaries, and handoff order clear during the first execution cycle.

This is a planning and dependency document. Module behavior still lives in `specs/`.

## Current Execution Boundary
- Build the shared DSP platform before broad plugin product work.
- Treat `Phantom FET` as the first end-to-end validation target.
- Keep `Phantom Console` and `Phantom Tube EQ` behind shared-core maturity gates.
- Keep intelligence advisory-only and downstream of trustworthy measurement outputs.

## Active Milestone Status

### Milestone 0: Spec Consolidation
Documented today:
- shared detector spec
- shared calibration spec
- shared oversampling spec
- shared validation harness spec
- `Phantom FET` gain-computer spec
- `Phantom Console` routing spec

Still incomplete against Milestone 0 exit criteria:
- no `Phantom Tube EQ` Phase 1 spec/acceptance document is present yet
- several pre-coding open questions remain unresolved in the implementation plan

### Milestone 1: Shared DSP Infrastructure
This is the next executable milestone for engineering, with the caveat that open questions must not be silently filled in by implementation.

## Scope Boundaries

### Shared Core
Owns:
- processing lifecycle
- audio and channel abstractions
- calibration state
- oversampling services
- shared metering and latency reporting
- state/preset contracts

Must not own:
- product-specific control laws
- plugin routing policy
- plugin-specific saturation flavor decisions

### Shared DSP
Owns:
- detectors
- gain computers
- nonlinear building blocks
- reusable filter primitives

Must not own:
- plugin shell code
- product-specific parameter mapping
- duplicate implementations of shared behavior inside plugin branches

### Plugin Layers
Own:
- graph composition
- parameter mapping
- plugin-facing state
- product-specific routing and metering modes

Must not:
- fork detector logic
- fork gain-computer logic
- define new calibration assumptions outside shared core

### Intelligence Layer
Owns:
- feature extraction
- rule-based recommendations
- plugin hint adapters

Must not:
- alter the direct audio path
- ship before measurement features are trustworthy

## Dependency Order

### Parallelizable Foundation Work
These branches can begin in parallel where practical:
- `core/dsp-engine`
- `core/oversampling-engine`
- `core/calibration-system`

### Hard Dependency Chain
These branches are ordered:
1. `dsp/detector-framework`
2. `dsp/gain-computer-framework`
3. `dsp/fet-engine`
4. `plugin/phantom-fet`

### Conditional Follow-On Work
- `plugin/phantom-console` may proceed only after shared dynamics blocks are stable and routing behavior remains deterministic.
- `plugin/phantom-tube-eq` may proceed only after passive interaction and transformer/tube makeup behavior have dedicated specs and reliable shared primitives.
- `ai/audio-analysis-engine` and `ai/mix-assistant` may proceed only after validation outputs are stable enough to support measurable recommendations.

## First Vertical Slice: Phantom FET
`Phantom FET` is the first integrated target because it exercises the highest-risk core interactions in a controlled graph:
- fast peak detection
- FET-style gain computation
- GR-linked nonlinear behavior
- oversampling-aware audio path
- calibration-aware operating levels
- measurable metering and validation hooks

If this slice is not reusable, the rest of the lineup will drift.

## Readiness Gates By Branch

### `core/dsp-engine`
Ready to proceed with:
- processing lifecycle
- channel/layout abstractions
- meter bus contracts
- shared state containers

Must not lock prematurely:
- plugin-facing parameter semantics

### `core/oversampling-engine`
Ready to proceed with:
- `1x`, `2x`, `4x` mode support
- latency-reporting API
- explicit graph-boundary integration contract

Blocked from final sign-off by:
- unresolved quality-tier and CPU-budget targets

### `core/calibration-system`
Ready to proceed with:
- reference-mode model
- conversion helpers
- per-plugin calibration profile API

Must remain aligned with:
- nonlinear onset representation
- metering thresholds

### `dsp/detector-framework`
Ready to proceed with:
- coefficient helpers
- peak detector
- RMS detector
- stereo linking hooks
- sidechain filter insertion points

Must not hard-code:
- plugin-specific threshold laws
- plugin-specific linking defaults

### `dsp/gain-computer-framework`
May proceed after detector framework is stable.

Must expose:
- dB-domain curve evaluation
- attack/release application
- plugin hook points for alternate modes

### `dsp/fet-engine`
May proceed after detector and gain-computer frameworks are stable and oversampling integration is defined.

Must preserve:
- explicit GR signal input
- all-buttons as a distinct mode
- oversampling-aware nonlinear behavior

### `plugin/phantom-fet`
May proceed only after:
- shared core lifecycle is usable
- detector and gain-computer contracts are stable
- minimal validation harness path exists

Still blocked from full product lock:
- final threshold/input control law remains unresolved in the implementation plan

## Open Questions That Must Stay Explicit
- exact oversampling quality tiers and CPU budget targets
- default stereo-link policy where multiple policies are available
- final threshold/input control law for `Phantom FET`
- exact `Phantom Console` routing permutations exposed in UI vs internal-only support
- golden-reference data format for validation snapshots

These are architecture-level blockers, not implementation details to improvise.

## Downstream Readiness
- DSP Lead may proceed on shared detector, gain-computer scaffolding, and FET-engine prerequisites once core interfaces are in place.
- Plugin Engineer may prepare `Phantom FET` graph composition only against stabilized shared interfaces.
- Validation Engineer may proceed on harness structure and acceptance hooks, but not baseline format finalization without resolving the open data-format question.
- UI Systems Engineer should remain downstream of stable parameter/state contracts for the active FET slice.
- AI Systems Engineer should remain downstream of validated measurement outputs.
