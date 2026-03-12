# Shared DSP Foundation

## Implemented Blocks
- `core/calibration/`: shared calibration reference state, profile registration, nominal/onset/clip queries, normalized drive helpers.
- `core/oversampling/`: explicit `1x`, `2x`, `4x` oversampling wrapper with per-mode latency reporting and per-block processor context.
- `core/metering/`: reusable level and gain-reduction meter ballistics.
- `dsp/detectors/`: peak and RMS detectors, linked/unlinked modes, sidechain filtering, debug taps, and a program-dependent scaffold.
- `dsp/gain/`: reusable dB-domain gain computer with hard, soft, and variable knee behavior plus attack/release application.
- `dsp/nonlinear/`: transformer, tube, VCA, FET, diode/zener, and Vari-Mu family scaffolds with calibration-aware drive context.
- `dsp/analog/`: deterministic component variance, white-noise injection, stereo crosstalk, and power-supply-memory primitives.

## Targeted Signal Behavior
- Detector timing remains sample-rate-derived and explicit across `44.1 kHz` through `192 kHz`.
- Calibration exposes nominal, nonlinear, soft-sat, and clip regions from a single shared reference model.
- Gain reduction is measured in dB and converted to reusable linear gain for plugin graphs.
- Nonlinear stages stay calibration-aware and expose telemetry for harmonic activity and saturation proximity.
- Oversampling is explicit per module and keeps latency queryable instead of hiding it inside plugin code.

## Assumptions And Approximations
- The oversampling engine uses a runtime-designed half-band FIR stage. Quality-tier selection and final CPU budget remain open architecture questions.
- Nonlinear families are reference production models or scaffolds, not final hardware-fitted captures.
- No default stereo-link policy is hidden in the detector API; linked mode requires an explicit policy.
- The unresolved `Phantom FET` threshold/input law is not implemented here. Plugin agents still own public control mapping.
- Golden-reference snapshot storage is not finalized here; only telemetry/data schemas are provided.

## Required Validation
- Build and run `cmake -S . -B build && cmake --build build && ctest --test-dir build`.
- Add measurement-level timing and alias checks before treating the oversampling and detector responses as release-ready.
- Capture per-plugin calibration profiles before plugin agents rely on nominal/onset thresholds in shipping products.
- Validate nonlinear families against target measurements before tuning product-specific saturation profiles.

## Plugin-Agent Integration Surface
- Plugin agents can register a calibration profile, query nominal/headroom state, and derive normalized drive/saturation values from `core/calibration/`.
- Dynamics-oriented plugins can use `dsp/detectors/` and `dsp/gain/` directly without re-implementing envelope logic.
- Nonlinear stages can be wrapped inside `core/oversampling/` and consume shared `DriveContext` plus calibration metadata.
- Console, FET, and Tube EQ branches can now integrate metering, crosstalk, noise, and component variance hooks from shared code instead of local one-offs.
