# Architecture: Phantom Mix Suite

## Overview
Phantom Mix Suite is a modular audio plugin suite. The architecture prioritizes separation of concerns, ensuring that DSP, UI, and Intelligence layers remain decoupled.

## Directory Structure & Ownership
| Folder | Owner | Purpose |
|--------|-------|---------|
| `docs/` | Product Architect | Human-readable docs, architecture overviews, designer guides. |
| `specs/` | Product Architect / DSP Lead | Formal implementation specifications, "The Law". |
| `research/` | DSP Research Agent | algorithm experiments, circuit studies, measurement notes. |
| `core/` | DSP Lead Agent | Shared infrastructure (framework, audio utils, analog modeling base). |
| `dsp/` | DSP Lead Agent | Pure processing blocks (detectors, gain-computers, nonlinear modeling). |
| `intelligence/`| AI Systems Engineer| Assistant engine, analysis, and advisory-only logic. |
| `ui/` | UI Systems Engineer | Design system, components, layouts, and themes. |
| `plugins/` | Plugin Engineer | integration of Core + DSP + UI into product form. |
| `tests/` | Validation Engineer | Unit, integration, regression, and validation tests. |
| `benchmarks/` | Validation / DSP Lead | CPU profiling, performance monitoring. |
| `assets/` | UI Systems Engineer | fonts, icons, graphics, backgrounds. |
| `presets/` | Plugin / AI Engineer | Factory presets and schemas. |
| `data/` | DSP Lead / AI Systems | Harmonic targets, calibration curves, measured responses. |
| `ci/` | Integration Engineer | CI/CD configurations, release pipelines. |
| `cmake/` | Integration Engineer | Build configuration helpers and presets. |

## Branching Strategy
All development branches must follow the pattern: `category/description`.

### Branch Categories
- `core/`: Shared infrastructure/engine logic.
- `dsp/`: Mathematical modeling and signal blocks.
- `ui/`: Interface and design system components.
- `ai/`: Intelligence and analysis engine.
- `plugin/`: Target plugin product integration.
- `feature/`: Cross-cutting features spanning multiple layers.
- `research/`: Experimental sanity checks and prototypes.
- `release/`: Preparation for production launch.
- `hotfix/`: Critical production repairs.

### Merge Rules
- **`main`**: Receives only from `release/*` or `hotfix/*`. Never merges directly from features.
- **`develop`**: Receives from `feature/*`, `plugin/*`, `core/*`, `dsp/*`, `ui/*`, and `ai/*`. Integration target.
- **`research/*`**: Sandbox branches, optionally merged into `dsp/*` once validated.
- **`release/*`**: Follow `develop → release/vX → main` flow. Merges back to `develop` after main merge.

## Enforcement
- If code disagrees with `specs/`, the code must be corrected.
- DSP blocks in `dsp/` must remain pure (framework agnostic).
- `ui/` must not contain audio processing logic.
- `intelligence/` should strictly be advisory (no audio-path side-effects).
