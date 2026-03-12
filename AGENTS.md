# AGENTS.md: Collaborative AI-Human Workflow

## Vision
Phantom Mix Suite is built through the synergy of human expertise and AI efficiency. This document outlines our collaborative roles and interaction protocols.

## Agent Roles
- **Antigravity (Lead AI Engineer / QA)**:
  - Repository management and directory structure enforcement.
  - Quality Assurance: Ensuring code matches `specs/`.
  - Automated testing and CI/CD oversight.
  - Initial DSP module drafting and UI scaffolding.

- **Human Lead (Architect / Product Suite Manager)**:
  - High-level architectural decisions and product vision.
  - Final approval on `specs/` and `MISSION.md`.
  - Critical listening and subjective evaluation of DSP models.

## Interaction Protocols
- **Structure First**: All new work must be pre-planned in `.agent/` artifacts.
- **Spec-Driven**: Coding follows the formal definitions in `specs/`.
- **Branching Discipline**: Work always occurs in the appropriate `core/*`, `dsp/*`, `ui/*`, `ai/*`, or `plugin/*` branches.
- **Commit Standards**: `type: short description` (e.g., `feat: implement fet gain compression in dsp/`).

## Project Status
Initialization phase complete. Moving to Core DSP development.
