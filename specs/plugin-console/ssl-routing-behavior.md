# Plugin Spec: Phantom Console SSL Routing Behavior

## Purpose
Define the routing logic, sidechain taps, and section-order behavior for `Phantom Console` in its SSL-inspired operating modes.

## Product Intent
Routing is not a convenience feature in Phantom Console. It is part of the sound. Section order and sidechain source selection must materially change dynamics behavior, tonal interaction, and workflow speed.

## Scope
This spec covers:
- section ordering
- pre/post routing modes
- sidechain source behavior
- dynamics and EQ interaction points
- gate/expander routing expectations

This spec does not define:
- exact EQ curve coefficients
- exact compressor detector equations
- UI styling

## High-Level Channel Path
Baseline graph blocks:
1. input trim / calibration
2. optional analog input stage
3. HPF / LPF
4. dynamics section
5. EQ section
6. routing matrix
7. output trim
8. optional noise and crosstalk layers

The plugin must support alternate ordering where required by mode.

## Core Routing States

### Required Main Order Modes
- `Dynamics -> EQ`
- `EQ -> Dynamics`

Behavior expectations:
- `Dynamics -> EQ`: compression responds more directly to raw incoming spectrum; later EQ can reshape tone without re-triggering the compressor
- `EQ -> Dynamics`: EQ boosts/cuts materially affect compressor trigger behavior and resulting groove/punch

This change must be audible, measurable, and not treated as a cosmetic reorder.

## Dynamics Section Breakdown
The dynamics section shall include:
- compressor
- gate / expander
- sidechain filtering controls
- optional filter/EQ feed into sidechain depending on routing mode

The gate/expander must remain fast, stable, and resistant to chatter when properly set.

## Sidechain Source Modes

### Required Options
- raw input sidechain
- filtered sidechain
- EQ-influenced sidechain
- future external sidechain hook placeholder

### Behavioral Rules
- when sidechain filter routing is engaged, detector responds to filtered content while audio path remains unchanged
- when EQ-to-sidechain mode is engaged, the EQ or selected filter contribution must alter detector stimulus without necessarily changing section order in the audible path
- sidechain source changes must be recallable and automation-safe

## Filter and EQ Interaction Requirements

### Filters in Audio Path
- HPF and LPF must affect audible signal according to selected routing order

### Filters in Sidechain
- filters may be routed into sidechain independently where supported
- this path is especially important for reducing low-frequency over-triggering

### EQ Variants
Required EQ families:
- `02` variant
- `242` variant

Routing expectation:
- both EQ variants must participate correctly in `EQ -> Dynamics` and sidechain-EQ modes
- switching EQ family must not break routing persistence or preset restoration

## Gate / Expander Routing Behavior
- gate / expander should derive control from the configured dynamics sidechain path
- hysteresis behavior must be preserved regardless of EQ order mode
- fast recovery settings must avoid obvious chatter on sustained near-threshold material
- routing changes must not invalidate hysteresis state

## Compressor Routing Behavior

### `Dynamics -> EQ`
Expected use:
- level control before tonal shaping
- more predictable compression independent of later tone boosts

Expected audible result:
- cleaner control behavior
- post-compression tone shaping can add presence without re-driving detector

### `EQ -> Dynamics`
Expected use:
- classic tone-into-compression interaction
- boosts in upper mids or lows can change grab, punch, and envelope response

Expected audible result:
- more color from section interaction
- user can intentionally shape compressor sensitivity with EQ decisions

## Sidechain Recommendation Hooks
Routing system must expose enough state for intelligence features to detect:
- low-frequency detector dominance
- likely pumping from bass-heavy trigger content
- likely gate chatter from aggressive filtering or threshold settings

## State Model Requirements
Persist at minimum:
- main routing order
- compressor sidechain source
- gate/expander sidechain source if separate
- sidechain filter enable/state
- EQ variant

Rules:
- preset restore must reconstruct signal graph order exactly
- automation between routing states must avoid invalid processor state or stale detector sources

## Validation Requirements

### Functional Tests
- verify signal graph order for `Dynamics -> EQ` and `EQ -> Dynamics`
- verify EQ boosts affect compressor control only in modes where routing dictates they should
- verify sidechain HPF reduces low-frequency-triggered compression while leaving audio-path low end intact when used as sidechain-only filter

### Dynamic Tests
- compare compressor GR traces with and without EQ driving the detector
- verify gate hysteresis remains stable across routing modes

### Regression Tests
- preset save/load for all routing combinations
- automation stress test when switching routing during playback

### Perceptual Checks
- routing changes must be easy to hear on drums, vocals, and bass-forward material
- plugin must maintain a clean SSL-family baseline, not excessive coloration, unless analog options are intentionally engaged

## Definition of Done
- Main routing modes are implemented and measurably distinct.
- Sidechain filtering and EQ-to-detector behavior work as specified.
- Gate/expander remains stable across routing states.
- Presets and automation preserve routing deterministically.
