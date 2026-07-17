# Develop Branch Guide

## Role
Central integration branch for all new features and fixes before they reach production (main).

## Structure Guidelines
- All code in this branch must be functional and pass basic unit tests.
- Maintain architectural compliance with the 21-folder structure.

## Merge Rules
- Receives merges from `feature/*`, `bugfix/*`, and `Update Version/*` branches.
- Merges into `main` only during release cycles.
