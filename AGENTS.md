# AGENTS.md: Collaborative AI-Human Workflow

## Vision
Phantom Mix Suite is built through the synergy of human expertise and AI efficiency. This document outlines our collaborative roles and interaction protocols.

## Agent Roles
- **Product Architect**:
  - Responsible for: system architecture, specs, and plugin design.
  - Folders: `docs/`, `specs/`.

- **DSP Lead**:
  - Responsible for: nonlinear engines, detectors, saturation models, oversampling.
  - Folders: `core/`, `dsp/`, `data/`, `benchmarks/`.

- **Plugin Engineer**:
  - Responsible for: plugin signal graphs, parameter mapping, host integration.
  - Folders: `plugins/`, `presets/`.

- **Validation Engineer**:
  - Responsible for: regression tests, harmonic measurements, timing verification.
  - Folders: `tests/`, `benchmarks/`.

- **AI Systems Engineer**:
  - Responsible for: mix assistant, audio analysis, recommendation engine.
  - Folders: `intelligence/`, `data/`, `presets/`.

- **UI Systems Engineer**:
  - Responsible for: plugin UI, design system, metering.
  - Folders: `ui/`, `assets/`.

- **Integration Engineer**:
  - Responsible for: CI/CD, build scripts, packaging.
  - Folders: `ci/`, `scripts/`, `cmake/`.

- **Antigravity (Lead AI Engineer / QA Specialist)**:
  - Repository management and directory structure enforcement.
  - Quality Assurance: Ensuring code matches `specs/`.
  - Automated testing and CI/CD oversight.

## Interaction Protocols
- **Structure First**: All new work must be pre-planned in `.agent/` artifacts.
- **Spec-Driven**: Coding follows the formal definitions in `specs/`.
- **Branching Discipline**: Work always occurs in the appropriate `core/*`, `dsp/*`, `ui/*`, `ai/*`, or `plugin/*` branches.
- **Commit Standards**: `type: short description` (e.g., `feat: implement fet gain compression in dsp/`).

## Project Status
Initialization phase complete. Moving to Core DSP development.
