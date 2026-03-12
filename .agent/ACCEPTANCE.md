# ACCEPTANCE CRITERIA: Phantom Mix Suite

## General Infrastructure Rules
- **Given** an initialized plugin, **When** standard time passes, **Then** all directories must conform perfectly to `ARCHITECTURE.md` avoiding overlaps between DSP, UI, and AI paths.
- **Given** the internal DSP calculation blocks, **When** interacting directly with feedback nodes, **Then** 64-bit double precision must be used to preserve math integrity.
- **Given** oversampling capabilities, **When** requested, **Then** developers must build them per-module (1x-8x scale based on nonlinear intensity) rather than universally enforcing high CPU costs.

## Core DSP Verification
- **Given** a non-linear tube or FET device, **When** the signal encounters hard transients, **Then** actual time-dependent gain control (attack/release coefficients) must govern saturation logic before mere waveshaping happens.
- **Given** passive topologies like a Pultec, **When** simultaneous LF boost and cut are engaged, **Then** phase shift and impedance (shelving/bandwidth interaction) must alter dynamically rather than stacking a static bell curve.
- **Given** independent blocks wired sequentially, **When** the components are stacked, **Then** loading and impedance interactions between stages (level-dependent changes) must occur where appropriate in `docs/` specifications.

## Analog Realism Checks
- **Given** identical plugin instances (L/R), **When** component variance is enabled, **Then** a deterministic randomized deviation (1-3% drift on Q and frequency centers) must spread the processing slightly without causing unpredictable panning.
- **Given** specific calibrated reference levels (e.g., -18 dBFS), **When** operating at nominal levels, **Then** plugins must be virtually transparent but linearly scale their odd/even harmonic generation precisely as headroom limits are reached.
- **Given** the power supply stage of a Vari-Mu, **When** significant compression is occurring, **Then** headroom must decrease simulating analog rail sag and subsequent transient recovery bloom.

## Intelligence Component Rules
- **Given** the AI analysis engine running globally, **When** scanning for peaks or harmonic density, **Then** its calculations must exclusively remain an advisory overlay, and never impose unprompted modifications or side-effects onto the audio path.
- **Given** elevated LF energy dominating a mix bus, **When** using intelligence inside Phantom Console, **Then** the engine should actively suggest engaging a sidechain HPF to control unwanted pumping.

## Sample Cases (TDD Workflows for Validation)
### 1. The SSL / API RMS Check
* **If** a heavy 1kHz input sine steps rapidly by 10dB into Phantom Console processing with a long release parameter,
* **Expect** the RMS envelope measurement to linearly smooth across the defined window length rather than causing discrete stair-stepping on the output waveform.

### 2. The Opto Peak Response
* **If** the T4-style detector inside Phantom Opto has sustained dense RMS exposure over a 3-second chorusing vocal stack,
* **Expect** a dynamically measurable memory-effect (delayed release window behavior) compared to the exact same compressor clamping down on a 50ms transient burst.

### 3. Harmonic Purity
* **If** any FET block's input stage is driven 12dB past its 0 VU calibration target,
* **Expect** THD to increase dramatically with noticeable 3rd/odd harmonic leaning alongside transient clamping, rather than just raw flat digital clipping.
