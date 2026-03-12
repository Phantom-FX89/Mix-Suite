# Plugin Acceptance Spec: Phantom Tube EQ

## Purpose
Define the product-level acceptance criteria for `Phantom Tube EQ` so the passive network, makeup stage, and transformer coloration are implemented around the intended musical result.

## Product Promise
Phantom Tube EQ must deliver broad passive contouring, low-end weight, silky top-end shaping, and gentle tube/iron sweetening without collapsing into a harsh or over-hyped EQ.

## Required Signal Path
The accepted product must include:
1. input transformer / pad
2. passive LC network
3. LF/HF interaction rules
4. tube makeup gain stage
5. output transformer
6. output trim

## Required User-Facing Features
- LF boost/cut interaction
- HF boost/atten interaction where product design exposes it
- makeup gain compensation behavior consistent with passive insertion loss
- output trim
- metering sufficient for input/output and saturation proximity if product policy includes it

## Audible Acceptance Criteria
- LF contour interaction must feel broad and musical rather than surgical
- top-end lift must stay silky rather than brittle
- passive insertion loss and makeup gain behavior must produce the expected weight/sweetening relationship
- nominal-level operation must remain elegant and usable
- driven operation may add tube/transformer sweetness but must not become fizzy or hyped

## Behavioral Acceptance Criteria
- LF boost/cut interaction must be both audible and measurable
- HF interaction must remain broad and musical across intended settings
- makeup stage should compensate insertion loss while also contributing gentle tone when driven
- calibration shifts must affect headroom and sweetening onset predictably

## Technical Acceptance Criteria
- passive-network interaction must remain stable across all supported sample rates
- transformer/tube stages must integrate with shared oversampling policy where required
- preset restore must preserve all interaction-sensitive states
- broad curves must match design intent closely enough for listening and measurement agreement

## Validation Requirements

### Linear / Filter Tests
- verify LF boost/cut interaction curves
- verify HF boost/atten interaction curves
- verify broad-shape behavior against target fitted responses

### Nonlinear Tests
- verify makeup/output stages remain subtle at nominal level
- verify increased harmonic sweetening under higher drive without harsh clipping

### Product Checks
- kick/bass: LF lift produces weight without uncontrolled mud by default
- vocals/acoustic: top-end lift adds air without brittle glare
- mix bus elements: broad shaping feels musical, not clinical

## Non-Acceptance Conditions
- behaves like a standard clean digital bell/shelf EQ with cosmetic tube labels
- low-end shaping becomes boomy without the expected passive contour feel
- top-end enhancement becomes harsh or brittle under normal use
- insertion loss compensation is disconnected from the expected makeup-stage behavior

## Definition of Done
- Phantom Tube EQ delivers passive-first contouring with believable tube/iron makeup behavior.
- LF and HF interactions are audible, measurable, and musically useful.
- The plugin feels like a shaping tool first, with harmonics as supporting character rather than the main event.
