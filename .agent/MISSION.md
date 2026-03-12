# MISSION: Phantom Mix Suite

## Objective
Establish and construct an elite, modular audio plugin ecosystem "Phantom Mix Suite," grounded in modern, intelligent DSP modeling rather than cosmetic retro cloning. The primary objective is to build plugins that accurately reproduce analog time-dependent gain control, nonlinear stage interaction, component variance, and level-dependent headroom mathematically, governed by an overarching AI assistance layer that analyzes strictly measurable signal conditions.

## In-Scope
- Shared DSP Core: Non-linear modeling algorithms (FET, Tube, VCA, Transformer, Vari-Mu), dynamic detectors, active/passive filters, noise, crosstalk, and oversampling frameworks.
- Phase 1 Plugins: Phantom Console (SSL 4000 E), Phantom FET (1176), Phantom Tube EQ (Pultec).
- Intelligence Layer: Analytical tools for crest factor, RMS, tilt, and density to generate setup hints, warnings, and presets.
- Validation: Deterministic harmonic profiling, transfer function matching, and level-dependent regression testing.
- UI/UX: Dark, modern, graphite aesthetics with precision metering and health checks (no faux-hardware scaling UI elements).

## Out-of-Scope
- Creating superficial clones solely matching static EQ curves.
- Modeling UI photorealism (faux screw heads, glowing tubes).
- Auto-mixing AI that makes hidden background decisions or manipulates the direct signal path without user input.

## Constraints
- **Performance**: Must remain extremely stable with low CPU load despite complex DSP logic. Oversampling is per-module, not globally enforced.
- **Precision**: 64-bit double precision used for all non-linear and feedback-sensitive internal blocks.
- **Audio Authenticity**: Time behavior and interaction between processing stages supersede pure saturation mathematics.
- **Framework Compliance**: Adhere strictly to Neutral OS file architecture, branching maps, and agent ownership boundaries to prevent overlapping code.

## Inputs / Outputs
- **Inputs**: DSP Architecture blueprints (`specs/suite/dsp-architecture-blueprint.md`), component specifications, testing targets (`data/harmonic-targets/`).
- **Outputs**: Reusable DSP libraries (`dsp/`, `core/`), fully functional VST3/AU compiled plugins (`plugins/`), test harnesses and validation graphs (`benchmarks/`).

## Ownership & Operational Expectations
- **Product Architect**: Maintains overall `specs/` and `docs/`.
- **DSP Lead**: Provides `core/` modeling logic and `dsp/` blocks.
- **Plugin Engineer**: Composites DSP blocks into UI frameworks (`plugins/`).
- **Validation Engineer**: Secures the math with automated tests in `tests/`.
- **AI Systems Engineer**: Builds advisory analysis algorithms in `intelligence/`.
- **UI Systems Engineer**: Designs elements located in `ui/`.

## Initial Milestone Plan
### Milestone 1: Shared DSP Infrastructure
- Focus: Framework, Calibration system, oversampling (`core/*`).
### Milestone 2: Core Modeling Engines
- Focus: Gain computers, Transformers, WDF filters (`dsp/*`).
### Milestone 3: First Plugins
- Focus: Phantom Console, Phantom FET, Phantom Tube EQ integrations.
### Milestone 4: AI Assistance
- Focus: Crest factor, analysis engines, and health meters.
