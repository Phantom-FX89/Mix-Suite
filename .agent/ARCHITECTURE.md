# Architecture: Phantom Mix Suite

## Overview
Phantom Mix Suite is a modular audio plugin suite. The architecture prioritizes separation of concerns, ensuring that DSP, UI, and Intelligence layers remain decoupled.

## Directory Structure
- **docs/**: Human-readable documentation, overviews, and guidelines.
- **specs/**: Formal implementation specifications (The Law).
- **research/**: Experimental sandbox for new models and algorithms.
- **core/**: The engine room (framework, audio utils, analog modeling base).
- **dsp/**: Pure mathematical processing blocks (Gain computers, detectors, filters).
- **intelligence/**: AI assistant, analysis rules, and recommendations.
- **ui/**: Design system and shared visual components.
- **plugins/**: Product-specific integrations of Core + DSP + UI.

## Branching Strategy
| Branch Category | Scope | Folders |
|-----------------|-------|---------|
| `main` | Production Stable | All (Production ready) |
| `develop` | Integration / Next Release | All (In-progress) |
| `core/*` | Core framework/engine | `core/`, `juce/`, `tests/unit/`, `specs/shared-dsp/` |
| `dsp/*` | Mathematical modeling | `dsp/`, `tests/unit/`, `benchmarks/dsp/`, `data/` |
| `ui/*` | Interface & Design | `ui/`, `assets/ui/`, `docs/ui-guidelines/` |
| `ai/*` | Intelligence & Analysis | `intelligence/`, `data/`, `presets/`, `specs/intelligence/` |
| `plugin/*` | Target plugin product | `plugins/<name>/`, `presets/`, `tests/integration/` |
| `research/*` | Sandbox / Experiments | `research/`, `benchmarks/`, `data/` |
| `feature/*` | Cross-cutting features | Spans multiple layers |
| `release/*` | Stability for launch | `plugins/`, `presets/`, `docs/`, `ci/` |

## Enforcement
- If code disagrees with `specs/`, the code must be corrected.
- DSP blocks in `dsp/` must remain pure (framework agnostic).
- `intelligence/` should not affect the direct signal path significantly (chaos control).
