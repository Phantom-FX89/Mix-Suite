# Plugin Acceptance Spec: Phantom FET

## Purpose
Define the product-level acceptance criteria for `Phantom FET` so implementation, tuning, and validation can converge on a shared target before coding expands.

## Product Promise
Phantom FET must deliver extremely fast transient control, aggressive forwardness, and gain-reduction-linked harmonic excitement without turning into a generic distortion box.

## Required Signal Path
The accepted product must include:
1. input transformer model
2. input gain
3. peak detector / attack-release engine
4. FET gain reduction stage
5. GR-dependent saturation stage
6. output amplifier / transformer stage
7. output trim

## Required User-Facing Features
- standard ratio modes: `4:1`, `8:1`, `12:1`, `20:1`
- all-buttons mode
- attack control
- release control
- input and output level control
- meter mode support
- oversampling quality selection if exposed by product policy

## Audible Acceptance Criteria
- compression must feel extremely fast and assertive
- transients must become more forward or flattened depending on attack setting
- distortion must increase under gain reduction, not only when input is driven harder
- all-buttons mode must sound materially more explosive and unruly than `20:1`
- light to moderate compression at nominal calibration must remain musically useful

## Behavioral Acceptance Criteria
- attack range must provide clearly different transient behavior across its span
- release range must provide clearly different recovery behavior across its span
- ratio modes must not collapse into the same effective feel
- meter switching must not affect audio behavior
- calibration changes must shift headroom/drive behavior predictably

## Technical Acceptance Criteria
- detector and gain computer must remain stable across all supported sample rates
- GR-linked nonlinear growth must be measurable in validation harness output
- oversampling must reduce objectionable aliasing under heavy drive conditions
- preset restore must preserve ratio mode, all-buttons state, timing controls, and metering mode
- automation must not produce invalid gain states or unstable transitions

## Validation Requirements

### Dynamic Tests
- burst tests across attack settings
- release recovery comparison across release settings
- transient retention comparison between slowest and fastest attack settings

### Nonlinear Tests
- harmonic growth at fixed input with increasing GR
- harmonic growth comparison between `20:1` and all-buttons mode
- aliasing comparison at `1x` vs production oversampling mode

### Product Checks
- drums: clearly audible grab and excitement
- vocals: usable fast control without immediate harsh collapse
- bass: strong clamp possible without unstable pumping

## Non-Acceptance Conditions
- sounds like a generic saturator with compression attached
- all-buttons mode behaves like only a higher ratio
- heavy GR dulls everything without the expected edge/excitement
- nominal-level operation is already overly distorted

## Definition of Done
- Phantom FET is measurably fast, GR-reactive, and sonically distinct from cleaner compressor families.
- Mode differences are audible and testable.
- Product behavior matches the FET-family intent described in the suite blueprint.
