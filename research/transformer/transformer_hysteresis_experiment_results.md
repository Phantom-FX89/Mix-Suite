# Transformer Hysteresis Experiment Results

Status: synthetic prototype results, not hardware-matched validation

Prototype source: `research/transformer/transformer_hysteresis_experiment.cpp`

Compile and run command:

```bash
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror research/transformer/transformer_hysteresis_experiment.cpp -o /tmp/transformer_hysteresis_experiment && /tmp/transformer_hysteresis_experiment
```

Test conditions:
- sample rate: `48 kHz`
- tone test: `1 kHz` sine at `0.55` linear amplitude
- LF burst test: `50 Hz` burst at `0.75` linear amplitude
- candidate A: static contour plus saturator
- candidate B: contour plus bounded memory term

## Results

| Metric | Candidate A | Candidate B |
| --- | --- | --- |
| THD (%) | `7.645` | `8.331` |
| 2nd harmonic (dBr) | `-53.333` | `-34.828` |
| 3rd harmonic (dBr) | `-22.376` | `-21.848` |
| LF drift over burst (dB) | `-0.010` | `0.170` |
| Recovery to 10% memory (ms) | `0.021` | `184.500` |
| CPU (ns/sample) | `6.473` | `11.151` |
| Relative CPU cost (B/A) | `1.000x` | `1.723x` |

## Interpretation
- Candidate B produces the first meaningful memory-like recovery behavior in this branch's experiments. Candidate A effectively has none.
- Candidate B increases LF drift modestly, which is the expected direction for a bounded memory approximation that lets sustained low-end content change the operating state.
- Candidate B shifts harmonic balance materially by raising even-order content without a large change in third harmonic magnitude. That is a useful sign for "iron" character instead of only harder clipping.
- The measured CPU increase for Candidate B is about `1.72x` relative to Candidate A in this unoptimized standalone build. That is significant but still within a plausible shared-DSP research envelope.

## Conclusion
These first-pass synthetic results support the recommendation in `hysteresis-notes.md`: Candidate B is the most practical next production target. The next research gap is not whether a memory term helps. It does. The remaining questions are how much memory is musically useful, how aliasing behaves under oversampling, and whether hardware references justify a more physical model later.
