# Shared DSP Spec: Detector Framework

## Purpose
Define the detector subsystem used by dynamics processors across Phantom Mix Suite. This framework must provide predictable timing, calibration-aware level handling, sidechain shaping, and reusable APIs for plugin-specific gain computers.

## Design Goals
- Preserve target-family timing behavior before saturation embellishment.
- Remain sample-rate stable from 44.1 kHz to 192 kHz.
- Support feed-forward and feedback-derived topologies.
- Expose detector output in both linear and dB domains.
- Support internal mono, stereo, and dual-mono operation.
- Remain deterministic under automated tests.

## Scope
This spec covers:
- RMS detector
- Peak / rectified detector
- feedback source support
- sidechain pre-filtering
- detector linking behavior
- smoothing and coefficient calculation
- test requirements

This spec does not define:
- plugin-specific ratio laws
- plugin-specific makeup gain policy
- audio-path saturation stages

## Required Public Interface

### Lifecycle
Each detector implementation shall support:
- `prepare(sampleRate, maxBlockSize, channelMode)`
- `reset(initialLevel)`
- `setTiming(params)`
- `setSidechainConfig(config)`
- `processSample(input)` or block-equivalent processing

### Outputs
Each detector shall provide:
- linear envelope output
- dB envelope output
- pre-filtered sidechain tap output for debugging/tests
- current attack and release coefficient state for validation visibility where practical

## Channel Modes
- `mono`: single envelope
- `stereo-linked`: one shared control signal derived from both channels
- `stereo-unlinked`: independent channel envelopes
- `dual-mono`: identical implementation to unlinked with separate control exposure

## Timing Coefficient Rules
All time constants shall be derived from sample rate using exponential smoothing.

Recommended form:

```text
coeff = exp(-1 / (timeSeconds * sampleRate))
```

Rules:
- attack and release behavior must remain materially equivalent across all supported sample rates
- very short attack times must not denormalize or produce NaN/Inf states
- if time is zero in UI terms, the implementation shall clamp to a safe minimum internal time
- coefficient recalculation must be smoothing-safe when parameters are automated

## Level Domain Rules
- detector input may be linear audio sample magnitude or filtered sidechain magnitude
- dB conversion must clamp to a minimum floor before `log10`
- detector framework must define a shared minimum floor constant for silence handling
- calibration metadata shall be available to downstream processors but detector core must not hard-code plugin-specific thresholds

## Detector Types

### RMS Detector
Primary use:
- SSL-style compressor sections
- API-style bus detection

Base form:

```text
rms_sq[n] = (1 - a) * x[n]^2 + a * rms_sq[n-1]
rms[n] = sqrt(rms_sq[n])
```

Requirements:
- expose averaging window or equivalent timing parameter
- support optional attack/release post-smoothing if plugin behavior requires it
- support pre-detector sidechain filtering
- output shall be stable and monotonic for steady-state sine input

Implementation notes:
- use double precision internally
- clamp negative roundoff before square root
- when linked stereo mode is active, combine channels using max or energy-sum policy selectable by plugin graph

### Peak / Rectified Detector
Primary use:
- 1176 / FET family
- gate / expander sections
- fast transient processors

Base form:

```text
rect[n] = abs(x[n])

if rect[n] > env[n-1]:
    env[n] = attack_coeff * env[n-1] + (1 - attack_coeff) * rect[n]
else:
    env[n] = release_coeff * env[n-1] + (1 - release_coeff) * rect[n]
```

Requirements:
- support extremely fast attack values without instability
- remain well-behaved at 192 kHz
- optional hold stage for gate/expander variants
- optional peak emphasis mode for plugin-specific voicing

### Program-Dependent Detector Hook
Primary use:
- opto and future program-dependent processors

Requirements:
- framework must allow a detector implementation with internal memory state beyond simple attack/release
- state exposure shall support test harness inspection
- accumulated exposure and multi-stage release behavior must be representable

This framework branch may initially ship as an interface plus reference scaffold rather than a complete LA-2A style model.

### Feedback Topology Support
Purpose:
- allow detector source to be taken from pre-gain or post-gain signal path

Requirements:
- detector API must allow the plugin graph to choose source position
- switching source position must be safe under automation and preset restore
- tests must verify materially different control behavior between feed-forward and feedback source positions

## Sidechain Pre-Filtering

### Required Filter Modes
- high-pass filter
- tilt / emphasis filter
- bell or broad EQ insertion point for plugin-defined modes
- bypass

### Design Rules
- filters may be implemented as lightweight IIR sections
- filter parameters must be smoothable
- sidechain path must not contaminate audio-path phase response
- plugin graph defines exact filter curves, but detector framework must supply reusable insertion points

## Stereo Linking Rules
- linked mode shall support at least one of the following policies:
  - max of absolute envelopes
  - energy-sum derived envelope
- link policy must be chosen explicitly by plugin graph, not implicitly hidden
- future extension should permit variable linking coefficient, but initial framework may use fixed linked/unlinked states

## Parameter Smoothing Rules
- attack, release, and detector filter parameters must be smoothed at control rate or sample rate where needed
- stepped UI values may remain stepped visually while internal coefficient transition avoids zippering
- bypassing sidechain filters must avoid large discontinuities where possible

## Numerical Stability Requirements
- no denormals
- no NaN/Inf propagation
- silence input must decay toward floor predictably
- reset state must be deterministic
- all processing should prefer double precision internally

## Debug and Validation Visibility
Detector framework should expose optional test/debug taps for:
- sidechain post-filter signal
- raw rectified or RMS-squared state
- final envelope linear value
- final envelope dB value

These taps may be compiled out or disabled in release builds if needed.

## Test Requirements

### Unit Tests
- coefficient calculation across all supported sample rates
- attack step response for representative time settings
- release decay response for representative time settings
- silence handling and floor clamping
- reset determinism

### Integration Tests
- stereo linked vs unlinked behavior
- sidechain HPF materially reducing low-frequency-triggered envelope rise
- feedback source producing distinct envelope path relative to feed-forward source

### Measurement Tests
- burst-based timing verification
- stepped sine detector consistency
- sample-rate invariance within acceptable tolerance

## Initial Implementation Breakdown
1. Shared detector base types and enums
2. Coefficient utilities and level-domain helpers
3. Peak detector implementation
4. RMS detector implementation
5. Stereo linking and sidechain filter insertion support
6. Feedback-source routing hooks
7. Validation/debug taps

## Definition of Done
- RMS and peak detector families are implemented and tested.
- Sidechain filtering and stereo linking work through shared interfaces.
- Detector timing remains stable across sample rates.
- Plugin teams can compose detector output into gain computers without rewriting envelope logic.
