# Shared DSP Spec: Nonlinear Modeling Families

## Purpose
Define the shared nonlinear model families used across Phantom Mix Suite so developers can implement device-specific coloration without collapsing the entire product line into one generic saturator.

## Design Goals
- Keep each analog family behaviorally distinct.
- Make nonlinear growth level-dependent and calibration-aware.
- Preserve stage interaction, not just isolated transfer curves.
- Support oversampling and validation from the start.
- Provide reusable interfaces while allowing plugin-specific tuning.

## Scope
This spec covers:
- transformer model family
- tube model family
- VCA model family
- FET model family
- diode / zener model family
- vari-mu model family
- shared API expectations
- calibration, oversampling, and validation requirements

This spec does not define:
- plugin graph routing
- detector equations
- UI control semantics

## Shared Nonlinear Architecture Rules

### Core Requirements
- every nonlinear stage must accept calibration-aware drive context
- every nonlinear stage must declare its preferred oversampling policy
- every nonlinear stage must expose parameters needed for measurement and debug capture
- every nonlinear stage must remain stable at all supported sample rates
- every nonlinear stage must be musically useful at nominal operating level

### Required Shared Inputs
Each model family should be able to consume some or all of:
- input sample or block
- calibration profile
- normalized drive amount
- gain reduction state where relevant
- optional memory/state term
- sample-rate and oversampling context

### Required Shared Outputs
- processed audio
- optional harmonic activity estimate or internal drive telemetry
- optional debug transfer sample data for validation harness use

### Domain Rules
- use double precision internally where feedback or high curvature makes it worthwhile
- clamp or guard all nonlinear domain operations against NaN/Inf
- define silence and reset behavior explicitly

## Family 1: Transformer Model

### Sonic Intent
Capture:
- low-frequency saturation
- mild phase shift and contouring
- odd/even harmonic generation depending on level and asymmetry
- soft dynamic compression near saturation
- optional slow magnetic memory effect

### Minimum Production Model
Required structure:
- minimum-phase contour stage
- soft saturator with asymmetry support
- optional slow memory term

Suggested practical form:

```text
y[n] = g * tanh(k * x[n] + b * m[n])
m[n] = alpha * m[n-1] + (1 - alpha) * x[n]
```

Implementation rules:
- contour stage should provide family-appropriate LF weight and top-end shaping
- memory term must remain subtle and bounded
- input and output transformer profiles must be separately configurable where plugin graph requires both

### Oversampling Policy
- default `2x`
- support `4x` when used in higher-drive stages

### Validation Targets
- harmonic growth vs level
- LF saturation behavior
- transfer curve stability
- no exaggerated fuzz at nominal level

## Family 2: Tube Model

### Sonic Intent
Capture:
- soft clipping
- asymmetry-driven even-order content
- headroom compression before clipping
- dynamic bloom under drive

### Base Model Options
Acceptable first implementation paths:
- polynomial triode-style waveshaper
- simplified physically informed static tube relation wrapped by gain staging

Suggested waveshaper form:

```text
y = a1*x + a2*x^2 + a3*x^3 + a4*x^4 + a5*x^5
```

Rules:
- asymmetry control must be explicit
- different tube personalities may be exposed as coefficient sets or profile tables
- makeup-stage tube tone and overt drive-stage tube tone should be separable where plugin architecture requires it

### Dynamic Extensions
- optional bias shift
- optional PSU sag coupling
- optional HF softening under stress

These may begin as hooks or parameters if the first delivery focuses on stable static-plus-light-dynamic behavior.

### Oversampling Policy
- default `2x`
- `4x` for driven tube stages or dense serial chains

### Validation Targets
- asymmetry-driven harmonic balance
- smooth clipping onset
- level-dependent bloom without oscillatory instability

## Family 3: VCA Model

### Sonic Intent
Capture:
- mostly transparent gain control
- slight firming under gain reduction
- very low baseline harmonic footprint

### Base Model
Required behavior:
- linear gain control is primary
- optional small nonlinear term may increase with gain reduction

Suggested form:

```text
y = g * x + c1 * GR * x^3
```

Rules:
- nonlinear term must remain subtle by default
- control law stability matters more than color quantity
- VCA path must not impersonate transformer or tube behavior

### Oversampling Policy
- native or `2x` depending on associated coloration stage

### Validation Targets
- clean baseline at nominal level
- predictable GR behavior
- low distortion floor relative to other model families

## Family 4: FET Model

### Sonic Intent
Capture:
- speed
- edge and brightness under control action
- gain-reduction-dependent distortion
- increased forwardness on transients

### Base Model
Required structure:
- input drive component
- GR-linked drive component
- asymmetric fast-clamping waveshaper or rational saturator

Suggested forms:

```text
drive = baseDrive + k1 * inputLevel + k2 * gainReduction
y = outputGain * satFET(inputGain * x, drive)
```

Possible saturator examples:

```text
y = x / (1 + a * abs(x))
```

Rules:
- distortion must increase with gain reduction, not only input level
- all-buttons-capable paths must expose mode hooks beyond simple ratio changes
- transient edge must remain audible when settings are appropriate

### Oversampling Policy
- default `4x`

### Validation Targets
- GR-linked harmonic growth
- transient-forward feel under moderate compression
- materially different response under alternate/all-buttons mode

## Family 5: Diode / Zener Model

### Sonic Intent
Capture:
- semiconductor bite
- sharper limiting texture than tube
- rounded but assertive edge appropriate to TG-style behavior

### Base Model
Acceptable initial approach:
- diode-like threshold conduction curve with smoothing

Suggested form:

```text
y = sign(x) * (1 - exp(-k * abs(x)))
```

Rules:
- control timing and audio-path edge must be independently tunable where plugin architecture needs both
- edge may be sharper than tube but must remain musical
- avoid brittle aliasing by requiring oversampling for higher-drive use cases

### Oversampling Policy
- default `4x`

### Validation Targets
- sharper knee than tube family
- stable symmetry/asymmetry policy depending on target device
- strong alias reduction relative to native mode

## Family 6: Vari-Mu Model

### Sonic Intent
Capture:
- bias-dependent smooth gain change
- dense but expensive-feeling reduction
- feedback-linked movement
- long-tail release personality

### Base Model Strategy
Acceptable initial architecture:
- hybrid model combining fitted nonlinear transfer behavior with feedback-safe control structure

Possible implementation options:
- fitted tube-data transfer curves
- simplified stateful nonlinear stage in a feedback-safe block
- surrogate transfer with release-program support

Rules:
- feedback topology and time behavior matter more than raw distortion quantity
- release program behavior must be represented explicitly
- avoid turning vari-mu into a generic tube saturator with compression on top

### Oversampling Policy
- default `4x`
- support `8x HQ` in future if justified by aliasing/performance analysis

### Validation Targets
- smooth dense gain reduction
- release-program distinction
- subtle harmonic richness without cheap crunch

## Shared Parameterization Rules

### Profile-Based Design
Each model family should support profile objects or parameter sets that define:
- baseline drive
- asymmetry
- contouring amount
- memory influence if applicable
- headroom or onset offsets relative to calibration

This allows one family implementation to support multiple plugin personalities without copying code.

### Clean vs Driven Regions
Every family must define behavior bands:
- below nominal
- nominal operating region
- nonlinear onset region
- soft saturation region
- clip protection or hard limit region

These bands must be grounded in the shared calibration system.

## Interstage Interaction Rules
- plugin graphs may place contouring, loading approximation, or gain staging before and after nonlinear blocks
- nonlinear families must not assume they are the only colored stage in the chain
- if multiple nonlinear blocks are adjacent, graph design should consider a shared oversampled island where appropriate

## Debug and Telemetry Expectations
Where practical, nonlinear models should expose:
- effective drive amount
- normalized saturation proximity
- GR-linked modulation amount for FET/VCA cases
- optional memory-state value for transformer/tube extensions

These outputs are for validation and intelligence support, not necessarily end-user display.

## Validation Requirements

### Unit Tests
- transfer curve sanity
- reset determinism
- parameter clamping and state stability

### Measurement Tests
- THD+N vs level
- harmonic spectrum at multiple drive points
- IMD tests for representative families
- alias comparison across oversampling modes

### Integration Tests
- calibration-aware onset shift
- interaction with oversampling wrapper
- interaction with plugin graph gain staging

### Perceptual Checks
- nominal-level behavior remains usable
- family differences are easy to hear on appropriate source material
- driven behavior reflects target family identity rather than generic saturation

## Initial Implementation Order
1. transformer model level-1 production implementation
2. FET model with GR-linked distortion hooks
3. tube model for makeup and sweetening stages
4. VCA model for cleaner dynamics products
5. diode / zener model for TG-family work
6. vari-mu model for later Phase 2 products

## Definition of Done
- Shared nonlinear family spec exists for all required analog behaviors.
- Each family has sonic intent, base math direction, oversampling policy, and validation targets.
- Developers can implement reusable model families without collapsing plugin identities.
- Calibration and oversampling dependencies are explicit before coding begins.
