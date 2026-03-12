# Transformer Hysteresis Research Note

Status: pre-production recommendation

Branch context: `research/transformer-hysteresis`

## Relevant Goals Extracted From `implementation plan.md`
- deliver a practical transformer stage with contouring, optional memory term, and calibration-sensitive onset
- keep the stage oversampling-aware and validation-friendly
- support `Phantom FET` and later `Phantom Tube EQ` without forking transformer logic per plugin
- validate transfer curves, harmonic growth vs level, LF saturation behavior, aliasing tradeoffs, and CPU cost

## Scope Assumption
The repository does not yet contain transformer measurements, benchmark harness code, or hardware capture data. This note therefore recommends the safest modeling path that fits the current specs and branch structure. It does not claim hardware-verified accuracy yet. A standalone synthetic prototype and first-pass comparison now exist in `research/transformer/transformer_hysteresis_experiment.cpp`.

## 1. Analog Behavior Being Studied
Transformer behavior for Phantom Mix Suite should capture:
- low-frequency saturation and mild dynamic compression as level rises
- subtle odd/even harmonic balance changes with asymmetry and level
- small amount of magnetic memory or hysteresis so repeated LF energy does not behave like a purely static waveshaper
- family-appropriate contouring instead of flat distortion
- calibration-sensitive onset so the same model behaves differently under `-20`, `-18`, and `-14 dBFS` nominal references

The main research question is how much dynamic hysteresis is worth modeling before CPU, stability, and tuning complexity stop paying for themselves.

## 2. Candidate Modeling Approaches

### Candidate A: Static Contour Plus Static Saturator
Structure:
- minimum-phase contour filter
- asymmetric soft clipper such as `tanh`, `atan`, or rational saturator
- calibration-aware drive mapping

Strengths:
- easiest to tune and stabilize
- lowest CPU cost
- clear fit for first-pass transfer-curve validation

Weaknesses:
- no real memory behavior
- LF drive feels generic under sustained energy
- tends to sound like "EQ plus saturator" rather than iron under repeated low-end stress

Best use:
- baseline comparison only

### Candidate B: Static Core Plus Bounded Memory Term
Structure:
- minimum-phase contour filter
- asymmetric saturator
- bounded slow state term driven by filtered input or internal drive

Suggested form:

```text
x_c[n] = contour(x[n])
m[n] = clamp(alpha * m[n-1] + (1 - alpha) * k_m * lp(x_c[n]), -m_max, m_max)
u[n] = drive * x_c[n] + bias + h * m[n]
y[n] = outGain * sat(u[n])
```

Strengths:
- captures the audible part of hysteresis that matters first: slight bias walk, LF compression, recovery memory
- stable if the state term is bounded and one-way coupled
- much easier to calibrate than a physical hysteresis solver
- CPU cost remains realistic for shared use in multiple plugins

Weaknesses:
- minor loops are only approximated
- not a literal magnetic-core model
- parameter tuning still needs listening plus measurement work

Best use:
- first production transformer family for this suite

### Candidate C: Dynamic Hysteresis ODE Approximation
Examples:
- Jiles-Atherton-inspired state update
- Preisach-like reduced memory model
- flux/bias ODE wrapped around a static saturator

Strengths:
- better remanence and loop behavior
- stronger claim to "real hysteresis" than Candidate B
- potentially better LF recovery realism under repeated burst tests

Weaknesses:
- parameter identification is hard without hardware data
- can become stiff or unstable at high sample rates and under automation
- higher CPU than the audible gain usually justifies at this project stage
- debugging and regression baselines become harder

Best use:
- continued research branch work, not the first production path

### Candidate D: WDF or Saturable-Inductor Network
Structure:
- transformer core or approximated magnetic element embedded in a WDF or related passive network solver

Strengths:
- strongest path for circuit interaction when transformer behavior is inseparable from surrounding passive network behavior
- attractive for future tube EQ or complex passive designs

Weaknesses:
- highest implementation complexity
- solver stability and tuning burden are much higher
- overkill for the current requirement of a reusable shared transformer stage
- difficult to justify before basic shared transformer behavior is benchmarked

Best use:
- targeted future study, especially if passive-network interaction becomes the bottleneck

## 3. Tradeoffs In Sound, Stability, and CPU

| Candidate | Sound | Stability | CPU | Repo Fit |
| --- | --- | --- | --- | --- |
| A. Static contour + saturator | Captures contour and soft saturation, misses memory | Excellent | Low | Good as baseline, weak as final shared family |
| B. Static core + bounded memory | Captures the most useful hysteresis cues without pretending to be full physics | Good if bounded and calibration-aware | Low-Medium | Best current fit |
| C. Dynamic hysteresis ODE | Better loop behavior and recovery nuance | Medium risk without careful integration | Medium-High | Research only for now |
| D. WDF magnetic network | Potentially most authentic circuit interaction | Highest risk and highest tuning burden | High | Premature for first production stage |

### Aliasing Tradeoff
- Candidate A aliases badly when driven hard because all character comes from a static nonlinearity.
- Candidate B still needs oversampling for the saturator, but the memory term itself mostly adds low-rate behavior rather than extreme HF alias products.
- Candidates C and D may generate less blatantly "clipper-like" spectra in some cases, but they do not remove the need for oversampling once strong nonlinearity is present.

### Recommended Oversampling Policy
- `2x` default for transformer stages used as modest coloration blocks
- `4x` optional when the transformer is pushed hard or lives inside a more aggressive chain
- keep the entire transformer stage inside one oversampled island for the first implementation to avoid state mismatch across rate boundaries

## 4. Recommendation
Recommendation: productionize Candidate B first.

Why:
- it satisfies the shared nonlinear spec already written for the suite
- it is strong enough to sound like transformer coloration rather than a generic saturator
- it keeps implementation risk low enough for reuse in both `Phantom FET` and `Phantom Tube EQ`
- it leaves room to upgrade the memory block later if hardware captures prove the approximation is insufficient

Recommended production structure:
1. minimum-phase contour block with profile-specific LF weight and top-end shaping
2. calibration-aware drive mapping derived from the shared calibration system
3. asymmetric saturator with separate input and output transformer profiles if needed
4. bounded slow memory term that influences bias or effective drive, not a fully recursive nonlinear solver
5. debug taps for transfer capture, harmonic-growth measurement, and internal memory-state observation

Recommended non-goals for first production:
- full Jiles-Atherton implementation
- WDF magnetic core solver
- parameter fitting against hardware data that does not yet exist in the repo

## 5. What The DSP Lead Can Safely Productionize Next
- create a shared transformer stage interface on `dsp/transformer-model` with explicit calibration, oversampling, asymmetry, and profile controls
- implement Candidate B with memory term bounds, reset rules, and debug taps from day one
- expose internal telemetry needed by the validation harness:
  - pre-contour level
  - post-contour level
  - memory-state value
  - output level
- validate at `44.1`, `96`, and `192 kHz`
- benchmark `1x`, `2x`, and `4x` modes before freezing defaults
- keep Candidate C and D isolated in research until there is hardware data or a strong passive-network interaction reason to justify the extra complexity

## Safe Initial Parameterization Guidance
- keep hysteresis subtle; if the memory term is obvious on midrange steady tones, it is probably too strong
- weight the memory driver toward LF content so the effect appears under sustained low-end energy rather than all material equally
- clamp internal state and all nonlinear inputs against `NaN` and `Inf`
- reset state deterministically so automation and preset recall remain testable

## Risks To Track
- a memory term that is too strong will sound like pumping or bias wobble instead of transformer memory
- profile-specific contouring can accidentally become EQ flavor disconnected from saturation onset
- differing oversampling boundaries across plugins may make the same transformer profile behave inconsistently
- without hardware or listening references, "more complex" may be mistaken for "more authentic"

## Bottom Line
The first transformer family should not chase full magnetic physics. It should ship as a contour-aware, calibration-aware saturator with a bounded LF-sensitive memory term. That is the highest-leverage point on the accuracy-versus-CPU curve for Phantom Mix Suite right now.
