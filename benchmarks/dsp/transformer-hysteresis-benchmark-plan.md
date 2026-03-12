# Transformer Hysteresis Benchmark Plan

Purpose: define the first measurement pass for comparing transformer-model candidates before any production lock-in.

## Candidate Set
- A: static contour plus static saturator
- B: static contour plus bounded memory term
- C: dynamic hysteresis ODE approximation, only if a prototype exists

## Test Matrix

### Sample Rates
- `44.1 kHz`
- `96 kHz`
- `192 kHz`

### Oversampling Modes
- `1x`
- `2x`
- `4x`

### Calibration Modes
- `0 VU = -20 dBFS`
- `0 VU = -18 dBFS`
- `0 VU = -14 dBFS`

### Signals
- `50 Hz` sine level sweep for LF saturation and memory behavior
- `1 kHz` sine level sweep for transfer-curve and harmonic growth capture
- `10 kHz` sine level sweep for alias sensitivity
- SMPTE two-tone for IMD growth under drive
- burst train with low-frequency emphasis for recovery and state-memory observation
- silence to driven to silence reset test for state determinism

## Measurements To Record
- transfer curve at multiple levels
- THD+N vs input level
- even/odd harmonic balance vs level
- alias-energy comparison across `1x`, `2x`, and `4x`
- gain compression at LF relative to midband
- memory-state recovery time after burst excitation
- CPU time per block and per sample
- reset determinism across repeated runs

## Comparison Rules
- compare candidates at matched output level
- compare the same profile and calibration mapping before comparing different profiles
- freeze baselines only after the DSP Lead approves the first acceptable candidate
- do not define hard pass/fail thresholds until a baseline implementation exists

## Recommended Exit Criteria
- Candidate B should materially outperform Candidate A on LF memory realism while staying close enough in CPU cost to remain a shared default
- `2x` should provide the default production balance unless alias measurements force `4x`
- Candidate C should only proceed if it produces a clearly audible and measurable improvement that justifies the extra CPU and stability risk
