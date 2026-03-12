# Plugin Spec: Phantom FET Gain Computer

## Purpose
Define the gain computer and control behavior for `Phantom FET`, inspired by the 1176 family while remaining compatible with Phantom Mix Suite shared DSP primitives.

## Product Intent
The Phantom FET gain computer must feel fast, forceful, and forward. It is not enough to expose ratio labels; the compressor must show materially different control feel and distortion behavior as ratio and gain reduction change.

## Scope
This spec covers:
- detector to gain-computer mapping
- ratio mode behavior
- attack/release control policy
- all-buttons mode behavior
- GR-dependent coloration hooks
- meter behavior requirements

This spec does not define:
- full transformer model internals
- UI art direction
- host/plugin shell behavior

## Signal-Path Context
Relevant local chain:
1. input transformer model
2. input gain
3. peak detector / attack-release engine
4. FET gain computer
5. GR-dependent saturation stage
6. output amplifier / transformer stage
7. output trim

The gain computer must be designed as part of this chain, not as an isolated static dB curve.

## Control Inputs
- input level into detector
- threshold reference derived from input/ratio interaction
- attack control
- release control
- ratio mode
- all-buttons state
- calibration reference from shared core

## Detector Input Assumptions
- source detector is the shared peak / rectified family
- detector output is expected in both linear and dB forms
- detector timing must be fast enough for FET-style transient control

## Gain Computer Behavior

### Domain
- operate in dB domain for static curve determination
- envelope application may occur in dB or linear gain domain, but output behavior must match dB-domain intent

### Static Curve
Use threshold `T`, input level `L`, ratio `R`, and optional knee width `W`.

Baseline hard-knee behavior:

```text
if L <= T:
    targetGR = 0 dB
else:
    targetGR = (1 - 1/R) * (T - L)
```

Rules:
- standard ratio modes should behave primarily as hard-knee or near-hard-knee compression
- extremely high ratio settings should approach limiting behavior without numeric discontinuity
- target gain reduction must be bounded to a safe maximum internal value

### Envelope Application

```text
if targetGR < currentGR:
    currentGR = attack_coeff * currentGR + (1 - attack_coeff) * targetGR
else:
    currentGR = release_coeff * currentGR + (1 - release_coeff) * targetGR
```

Rules:
- attack must be extremely fast relative to most suite compressors
- release must preserve excitement and avoid lazy or opto-like hang behavior
- release may include subtle program dependence only if it improves authenticity and remains testable

## Ratio Modes

### Standard Modes
Required modes:
- `4:1`
- `8:1`
- `12:1`
- `20:1`

Behavior goals:
- `4:1`: most open and mix-friendly; retains punch with moderate control
- `8:1`: firmer transient clamping and more obvious assertiveness
- `12:1`: near-limiting feel with stronger density and edge
- `20:1`: hard, aggressive, almost brickwall-style FET character while still musical

Implementation notes:
- do not only change the numeric ratio; each mode may shift knee hardness, recovery feel, or GR-to-distortion coupling slightly if needed to match target family behavior

## All-Buttons Mode
This mode must alter more than ratio.

Required behavioral shifts:
- steeper and less predictable effective control curve
- altered attack/release feel
- increased harmonic density
- stronger GR-dependent saturation growth
- more explosive forwardness and edge

Implementation guidance:
- allow alternate knee shape or blended multi-ratio response
- increase GR-to-distortion coefficient
- optionally bias detector or control-port nonlinearity for extra instability flavor, but remain stable and recallable

Definition of success:
- all-buttons mode sounds and measures materially different from `20:1`

## Attack and Release Control Policy

### Attack
- fastest setting must meaningfully flatten transients when pushed
- slowest setting must still remain within FET-family fast behavior
- parameter taper should favor useful resolution in the fast-control range

### Release
- fastest setting should create excitement and energy without pathological chatter under normal use
- slowest setting must still recover faster than opto/vari-mu families
- taper should favor musically relevant mid-fast settings

### Parameter Mapping Requirement
- UI values may follow target-hardware orientation if desired, but DSP mapping must be documented and testable
- any inverted knob logic must be normalized internally to avoid confusion in tests

## GR-Dependent Saturation Coupling
This is a core product requirement.

Rules:
- distortion amount must increase with gain reduction, not only with input drive
- gain computer must expose normalized GR state to the FET nonlinearity stage
- all-buttons mode must increase the influence of GR on distortion growth
- nominal-level operation at light GR should remain usable and not collapse into fuzz

Suggested interface:
- `grDb`
- `grNormalized`
- `ratioMode`
- `allButtons`

## Calibration Interaction
- threshold and GR behavior must remain meaningful under supported suite calibration references
- at nominal operating level, the plugin should allow controlled compression without immediate heavy distortion
- above nominal level, the gain computer and nonlinear path should combine into stronger edge and density

## Metering Requirements
Required meter modes:
- output level
- gain reduction
- optional input level if product UI supports it

Meter rules:
- GR meter must follow the applied control signal, not just detector target
- meter ballistics should feel responsive and readable
- meter mode switching must not affect audio behavior

## State and Automation Requirements
- ratio changes must avoid unstable clicks or invalid gain states
- attack/release automation must remain smooth
- all-buttons enable/disable must be click-safe where possible; if not perfectly clickless, transition must still be musically controlled
- preset recall must fully restore gain computer mode and timing state

## Validation Requirements

### Static Tests
- verify target GR curves for each ratio mode
- verify all-buttons differs from `20:1` in effective curve or dynamic response

### Dynamic Tests
- attack burst tests across control range
- release decay tests across control range
- transient retention comparison between slowest and fastest attack settings

### Nonlinear Coupling Tests
- harmonic growth at fixed input with increasing GR
- harmonic growth difference between standard mode and all-buttons mode

### Perceptual Checks
- compressor should remain exciting rather than dull under moderate compression
- heavy mode should sound aggressive, not merely louder

## Definition of Done
- Standard ratio modes and all-buttons mode are implemented and measurably distinct.
- Attack and release ranges produce authentic FET-family behavior.
- GR-dependent distortion coupling is exposed and tested.
- Metering reflects true applied gain reduction.
