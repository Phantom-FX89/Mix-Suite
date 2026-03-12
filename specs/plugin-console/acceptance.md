# Plugin Acceptance Spec: Phantom Console

## Purpose
Define the product-level acceptance criteria for `Phantom Console` so engineers can implement the channel strip around clear routing, dynamics, EQ, and workflow targets.

## Product Promise
Phantom Console must provide an SSL-inspired channel-strip workflow with clean baseline tone, fast mix-speed operation, strong routing significance, and stable dynamics/EQ interaction.

## Required Signal Path
The accepted product must include:
1. input trim / calibration
2. optional analog input stage
3. HPF / LPF
4. dynamics section
5. EQ section
6. routing matrix
7. output trim
8. optional crosstalk and noise layers

## Required User-Facing Features
- EQ variants: `02` and `242`
- compressor soft/hard knee behavior
- compressor log/linear release behavior
- dynamics pre/post EQ routing
- filters/EQ to sidechain options
- gate hysteresis behavior
- core metering for input, output, and gain reduction

## Audible Acceptance Criteria
- clean baseline tone at nominal level
- compressor must grab with RMS-style consistency rather than FET-style aggression
- EQ must support punchy upper-mid shaping and practical mix-speed decisions
- routing changes must materially change compression tone and response
- gate/expander must feel fast and stable without obvious chatter when set correctly

## Behavioral Acceptance Criteria
- `Dynamics -> EQ` and `EQ -> Dynamics` must be clearly distinct in real use
- sidechain filtering must reduce low-frequency over-triggering without altering audio-path low end when used in sidechain-only mode
- EQ variant switching must preserve consistent routing logic and state recall
- gate hysteresis must remain effective across routing states
- analog options must remain optional and not overwhelm the console's clean baseline identity

## Technical Acceptance Criteria
- routing state must serialize and restore deterministically
- automation between supported routing states must remain safe
- detector behavior must remain stable across sample rates
- crosstalk/noise options must be low-cost and disableable
- plugin graph must not duplicate shared detector or gain-computer logic

## Validation Requirements

### Routing Tests
- confirm `Dynamics -> EQ` and `EQ -> Dynamics` produce distinct GR behavior with identical source material
- confirm EQ boosts affect detector behavior only when routing allows it
- confirm sidechain HPF reduces bass-triggered pumping in appropriate modes

### Dynamics Tests
- verify compressor knee options are measurably distinct
- verify log/linear release modes are measurably distinct
- verify gate hysteresis reduces chatter around threshold

### Product Checks
- drums: punch and controllability without smeared routing behavior
- vocals: clear tone shaping and stable gate/compression interaction
- guitar/keys: fast workflow and predictable section interaction

## Non-Acceptance Conditions
- routing options are hard to hear or only cosmetic
- console tone is overly saturated at nominal level
- gate behavior chatters or collapses unpredictably
- EQ mode or sidechain mode breaks preset recall or routing persistence

## Definition of Done
- Phantom Console behaves like a real mix-speed channel strip rather than a loose collection of blocks.
- Routing, dynamics, and EQ interactions are audible, measurable, and stable.
- The product keeps a clean SSL-family baseline while supporting optional analog realism layers.
