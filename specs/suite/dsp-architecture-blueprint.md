# Phantom Mix Suite

## DSP Architecture Spec and Analog Modeling Blueprint

Version: 1.0
Audience: DSP engineer, JUCE/C++ plugin developer, technical product lead, validation engineer
Purpose: Translate the target analog hardware behavior into an implementable software architecture for Phantom Mix Suite.

---

# 1. Product Intent

Phantom Mix Suite is a modular plugin ecosystem inspired by elite analog hardware, but implemented as modern, intelligent DSP tools. The goal is not to create shallow cosmetic clones. The goal is to model the audible causes of analog behavior:

* time-dependent gain control
* nonlinear signal-path coloration
* passive and active filter interaction
* transformer and tube behavior
* level-dependent headroom and saturation
* component variance and analog imperfection
* intelligent analysis and assistance

Each plugin must sound excellent standalone, while sharing common DSP infrastructure and a common intelligence layer.

---

# 2. Product Lineup

## Phase 1 Plugins

### 2.1 Phantom Console
Primary inspiration: SSL 4000 E channel architecture
Future expansion mode: EMI TG channel personality mode
Core purpose: full channel-strip workflow, tone shaping, dynamics control, console-style routing, mix-speed decision making.

### 2.2 Phantom FET
Primary inspiration: 1176
Core purpose: fast transient control, edge, aggression, harmonic excitement, serial compression anchor.

### 2.3 Phantom Tube EQ
Primary inspiration: Pultec EQP-1A
Core purpose: passive EQ interaction, low-end weight and contour, broad musical top-end shaping.

## Phase 2 Plugins
### 2.4 Phantom Bus (API 2500)
### 2.5 Phantom Opto (LA-2A)
### 2.6 Phantom TG (EMI TG12345)
### 2.7 Phantom VariMu (Fairchild 670)
### 2.8 Phantom Passive EQ (Manley Massive Passive)

---

# 3. System-Level Architecture

Phantom Mix Suite shall be implemented as a shared DSP platform with plugin-specific signal graphs.

## 3.1 Shared Layers

### Layer A: Host / Plugin Infrastructure
Responsibilities: plugin instantiation, state serialization, preset system, automation-safe parameter smoothing, latency reporting, oversampling control, global metering bus, A/B compare, undo-safe parameter transactions.

### Layer B: Shared DSP Core
Responsibilities: detectors, gain computers, saturation/nonlinearity primitives, transformer/tube/FET/VCA models, filter structures, passive network models, crosstalk, noise, component variance, calibration system.

### Layer C: Plugin Graph Layer
Responsibilities: compose plugin-specific signal chains, implement routing models unique to target unit, map UI parameters to DSP graph.

### Layer D: Intelligence Layer
Responsibilities: signal analysis, source classification, settings recommendation, gain staging support, process-health warnings, plugin-chain hints.

### Layer E: Validation Layer
Responsibilities: deterministic test harnesses, regression tests, transfer-function matching, harmonic profiles, timing, perceptual AB workflows.

---

# 4. Non-Negotiable Modeling Principles

1. Do not model only static EQ curves and ratio knobs.
2. Prioritize time behavior before saturation cosmetics.
3. Model interaction between stages, not isolated blocks only.
4. Respect analog operating levels.
5. Preserve nonlinear growth with level and gain reduction.
6. Allow calibrated clean behavior where the hardware is clean.
7. Use shared primitives, but do not force one saturation model across all devices.
8. Implement plugin-specific control interactions faithfully.
9. All intelligence recommendations must be grounded in measurable signal conditions.
10. Each plugin must remain musically useful even with intelligence disabled.

---

# 5. Shared DSP Framework

## 5.1 Core Signal Types
Mono, Stereo, Dual-mono linked/unlinked.
Internal precision: 64-bit double precision for nonlinear and feedback-sensitive blocks; 32-bit float acceptable at plugin I/O boundary.

## 5.2 Sample Rate Support
44.1 kHz, 48 kHz, 88.2 kHz, 96 kHz, 176.4 kHz, 192 kHz.

## 5.3 Oversampling Framework
Per-module oversampling via polyphase up/down filters.
Required Defaults: Linear filters (1x-2x), Transformer/Tube (2x-4x), FET/Hard Nonlinear/Vari-Mu (4x-8x HQ).

## 5.4 Calibration System
Supported references: 0 VU = -18 dBFS, -20 dBFS, -14 dBFS.

## 5.5 Parameter Smoothing
Fast control smoothing for dynamics; slower smoothing for EQ; clickless bypass.

---

# 6. Analog Realism Engine

## 6.1 Component Variance Engine
Seeded deterministic randomization for center frequency drift, Q drift, timing drift.

## 6.2 Noise Engine
Shaped noise, thermal/tube hiss, hum. Level-dependent.

## 6.3 Crosstalk Engine
Low-level channel bleed and matrix-based crossfeed for consoles/bus processors.

## 6.4 Power Supply Behavior Engine
Signal-energy-dependent headroom reduction, sag, transient bloom tracking.

## 6.5 Interstage Loading Approximation
Impedance interaction between stages (essential for Pultec, Massive Passive).

---

# 7. Detector Framework
7.1 RMS Detector (SSL, API)
7.2 Peak / Rectified Detector (1176, TG)
7.3 Program-Dependent Opto Detector (LA-2A)
7.4 Feedback Detector Topology (Fairchild, API OLD)
7.5 Sidechain Pre-Filtering (HPF, EQ, spectral tilt).

---

# 8. Gain Computer Framework
Static Curve (dB domain), Envelope Application, Auto Makeup Gain, Mode-Specific Variants.

---

# 9. Nonlinear Modeling Families
9.1 Transformer Model Math (Hysteresis-like, LF sat, IMD-preserving)
9.2 Tube Model Math (Triode shaper, physically informed, level-dependent bias shift)
9.3 VCA Model Math (Clean gain control, tiny GR-linked nonlinear term)
9.4 FET Model Math (Speed, edge, GR-dependent clamping/saturation)
9.5 Diode / Zener Model Math (Soft diode, piecewise semiconductor bite)
9.6 Vari-Mu Model Math (Bias-dependent gain change, long-tail recovery)

---

# 10. Harmonic Target Profiles
SSL (Clean baseline, firm), TG (Denser mid, semiconductor edge), 1176 (Grit, odd-leaning under GR), LA-2A (Even-order warmth, soft contour), Fairchild (Dense, smooth glue), API (Punch first), Pultec/Massive Passive (Passive contour interacting with makeup sweetening).

---

# 11. Filter and EQ Architecture
11.1 Active EQ (IIR/biquad, SSL/TG utility)
11.2 Passive EQ (WDF where practical, interacting digital networks)
11.3 WDF Structures (Pultec, Massive Passive cores)
11.4 Shelf / Bell Interaction Support

---

# 12. Plugin-Specific Architecture Specs
(Reference full document for detailed chains for Phantom Console, FET, Tube EQ, Bus, Opto, TG, VariMu, Passive EQ).

---

# 13. Intelligence Engine Spec
Not auto-mixing AI; measurable analysis and recommendation engine covering RMS, peak, crest factor, transients, tilt, harmonic density. Outputs feature health meters, warnings, and suggested starting points/routing choices based on objective signal qualities.

---

# 14. Validation and Measurement Spec
Testing required: Linear sweeps, THD+N vs level, Intermodulation, Dynamic burst/step tests, state memory tests. Require physical perceptual workflows alongside data.

---

# 15. UI / UX Architecture Guidance
Visuals: Matte graphite, precise metering, minimal clutter, premium industrial. Stop retro hardware cosplay.

---

# 16. Build Order
Phase 0: Foundations (Shared core, calibration, OS, intelligence v1)
Phase 1: Console, FET, Tube EQ
Phase 1.5: Suite panel, cross-staging, shared presets
Phase 2: Bus, Opto, TG, VariMu, Passive EQ

---

# 17. Agent Roles for Codex / Antigravity
(See AGENTS.md for full assignment).

---

# 18. Developer Deliverables
1. Shared DSP library
2. Plugin signal graphs
3. Measurement harnesses
4. Oversampling
5. Intelligence AI API
6. Presets/state
7. Tuning docs.

---

# 19. Definition of Done
Complete when signal path is stable, time behavior matches target family, control interactions feel authentic, harmonics grow credibly with GR, measured curves align, aliasing is imperceptible, CPU is lean, and intelligence recommendations are musically valid.

---

# 20. Final Doctrine
Model movement before nostalgia, interaction before cosmetics. Calibration before hype. Hardware behavior before branding.
