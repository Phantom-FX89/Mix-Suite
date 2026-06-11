# Phantom Mix Suite

**Audio DSP Plugin Suite**  
High-fidelity mixing and mastering processors built with C++ and JUCE, featuring analog-modeled saturation, precision equalization, multi-band dynamics, and optimized real-time audio processing.

---

## Overview

Phantom Mix Suite is a collection of professional-grade audio plugins designed for modern mixing, mastering, sound design, live sound, and post-production workflows.

The suite is built around a modular DSP architecture that prioritizes audio quality, processing efficiency, and cross-platform reliability. Core processing components include parametric EQ, saturation, anti-aliasing, multi-band dynamics, high-resolution metering, and real-time parameter handling.

The project is developed in **C++** using the **JUCE** framework, with support planned for major plugin formats including **AU**, **VST3**, and **AAX**.

---

## Key Features

- **Parametric EQ**  
  Multi-band peaking and shelving equalization with real-time coefficient updates.

- **Linear-Phase Processing**  
  Phase-aware EQ and mastering workflows designed for transparent signal shaping.

- **Analog-Modeled Saturation**  
  Tube and transformer-style harmonic coloration using nonlinear DSP modeling.

- **Multi-Band Dynamics**  
  Frequency-selective compression and expansion for detailed dynamic control.

- **Anti-Aliasing Architecture**  
  Configurable oversampling and filtering for cleaner nonlinear processing.

- **High-Resolution Audio Processing**  
  Internal double-precision processing with 32-bit floating-point output support.

- **CPU-Conscious DSP Design**  
  Efficient filter structures, cache-aware processing, and future SIMD optimization support.

- **Cross-Platform Plugin Support**  
  Designed for macOS, Windows, and Linux through the JUCE framework.

---

## Architecture

Phantom Mix Suite is organized around reusable DSP primitives. Each processor is designed to be modular, testable, and efficient enough for real-time use in professional audio environments.

### Core DSP Modules

| Module | Purpose |
| --- | --- |
| Parametric EQ | Multi-band equalization using bi-quad filter topology |
| Saturation Engine | Harmonic coloration through nonlinear transfer functions |
| Dynamics Processor | Compression, expansion, and multi-band dynamics |
| Oversampling Engine | Anti-aliasing support for nonlinear processing |
| Metering System | Input/output RMS and peak metering |
| Plugin Wrapper | JUCE-based integration with supported plugin formats |

---

## Parametric EQ

The Parametric EQ module implements a 5-band equalizer using a cascade of bi-quad filters.

### EQ Capabilities

- 5 configurable EQ bands
- Peaking and shelving filter support
- Frequency, gain, and Q parameter control
- Per-channel filter state management
- RMS input/output metering
- Real-time coefficient updates
- Click-resistant parameter handling at block boundaries

### Example Structure

```cpp
class ParametricEQ {
public:
    static constexpr int NUM_BANDS = 5;

    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    std::array<FilterBand, NUM_BANDS> bands;
};
```

---

## Filter Architecture

Each EQ band uses a Direct Form II bi-quad filter implementation.

### Filter State

Each filter band maintains:

- Numerator coefficients: `b0`, `b1`, `b2`
- Denominator coefficients: `a0`, `a1`, `a2`
- Input history: `x1`, `x2`
- Output history: `y1`, `y2`

```cpp
struct FilterBand {
    double b0, b1, b2;
    double a0, a1, a2;

    double x1 = 0.0;
    double x2 = 0.0;
    double y1 = 0.0;
    double y2 = 0.0;

    inline double processSample(double input) {
        double output = b0 * input + b1 * x1 + b2 * x2
                      - a1 * y1 - a2 * y2;

        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;

        return output;
    }
};
```

---

## DSP Techniques

### Bi-Quad Filters

Each EQ band is implemented as a second-order recursive filter. This topology is widely used in audio DSP because it offers a strong balance of precision, stability, and computational efficiency.

A bi-quad filter uses:

- Feedforward taps: `b0`, `b1`, `b2`
- Feedback taps: `a1`, `a2`
- Two samples of input history
- Two samples of output history

This structure allows each band to operate efficiently while preserving enough flexibility for peaking, shelving, and other common EQ shapes.

### Coefficient Calculation

User-facing EQ controls are converted into filter coefficients using standard audio EQ formulas.

Core parameters include:

- Frequency
- Gain
- Q factor
- Sample rate
- Filter type

The coefficient update process includes:

1. Reading the current band configuration.
2. Calculating angular frequency.
3. Converting gain from dB to linear scale.
4. Updating normalized filter coefficients.
5. Applying the updated coefficients at safe processing boundaries.

### Cascaded Filter Processing

Multiple EQ bands are applied serially to create the final EQ response.

Processing flow:

1. Input sample enters Band 1.
2. Output of Band 1 feeds Band 2.
3. The chain continues through all active bands.
4. Final processed sample is written back to the audio buffer.

---

## Anti-Aliasing Strategy

Nonlinear processing, such as saturation, can generate harmonics above the Nyquist frequency. Phantom Mix Suite uses an anti-aliasing architecture designed to reduce unwanted foldback artifacts.

### Planned Anti-Aliasing Features

- Configurable 2x and 4x oversampling
- Low-pass filtering before downsampling
- Saturation-specific oversampling paths
- Latency compensation for oversampled processors
- Optional low-latency mode for tracking and live use

> Note: Linear-phase and oversampled processing may introduce latency depending on implementation. Low-latency modes should be treated as separate processing paths from linear-phase mastering modes.

---

## Real-Time Processing

### Parameter Mapping

```cpp
void ParametricEQ::setBand(int bandIndex, const EQBandConfig& config) {
    if (bandIndex >= 0 && bandIndex < NUM_BANDS) {
        bandConfigs[bandIndex] = config;
        bands[bandIndex].updateCoefficients(config);
    }
}

const EQBandConfig& ParametricEQ::getBand(int bandIndex) const {
    return bandConfigs[bandIndex];
}
```

### Processing Loop

```cpp
void ParametricEQ::processBlock(juce::AudioBuffer<float>& buffer) {
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch) {
        auto* data = buffer.getWritePointer(ch);

        for (int s = 0; s < numSamples; ++s) {
            float sample = data[s];

            for (auto& band : bands) {
                sample = static_cast<float>(band.processSample(sample));
            }

            data[s] = sample;
        }
    }
}
```

---

## Building

### Requirements

- C++17 or later
- CMake
- JUCE
- A supported compiler:
  - Clang
  - GCC
  - MSVC

### Build Commands

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

---

## JUCE Integration

Plugins inherit from `juce::AudioProcessor` and implement the standard JUCE plugin lifecycle.

### Core JUCE Methods

| Method | Purpose |
| --- | --- |
| `prepareToPlay()` | Initializes DSP state using the host sample rate and block size |
| `processBlock()` | Performs real-time audio processing |
| `releaseResources()` | Cleans up allocated processing resources |
| `getParameters()` | Exposes plugin parameters for UI and automation |

---

## Performance Targets

The following values represent target performance characteristics and should be validated through profiling during development.

| Metric | Target |
| --- | --- |
| CPU Usage | Approximately 2–5% per stereo instance at 48 kHz |
| Latency | 0 samples for minimum-phase/low-latency modes |
| Memory Usage | Approximately 512 KB per instance |
| Internal Precision | 64-bit floating-point processing |
| Output Precision | 32-bit floating-point audio output |

---

## Design Philosophy

Phantom Mix Suite is built around five core principles:

1. **Audio Quality**  
   Preserve clarity, depth, and detail while avoiding unwanted artifacts.

2. **Efficiency**  
   Keep processing lightweight enough for real-time mixing sessions.

3. **Usability**  
   Provide intuitive controls, useful metering, and responsive parameter changes.

4. **Modularity**  
   Keep DSP components reusable across processors and plugin types.

5. **Portability**  
   Maintain a cross-platform foundation through JUCE and CMake.

---

## Use Cases

- **Mixing**  
  Surgical EQ, tonal shaping, compression, and saturation for individual tracks.

- **Mastering**  
  Transparent processing, phase-aware EQ, loudness control, and final-stage enhancement.

- **Sound Design**  
  Harmonic coloration, creative distortion, and dynamic tone shaping.

- **Live Sound**  
  Low-latency processing for real-time applications.

- **Post-Production**  
  Restoration, enhancement, and precision control for recorded material.

---

## Implementation Notes

- Filter state should be maintained per channel to prevent cross-channel artifacts.
- DSP operations should use double precision internally where numerical stability matters.
- Parameter changes should be smoothed or applied at safe boundaries to prevent clicks.
- Metering should update at a UI-friendly rate rather than every sample.
- Oversampling should be applied selectively to nonlinear processors to manage CPU usage.
- Linear-phase processing and zero-latency processing should be treated as separate operating modes.

---

## Roadmap

Planned future enhancements include:

- GPU-accelerated processing for selected multi-band operations
- Machine learning-assisted mastering suggestions
- Sidechain filtering for dynamics and EQ processors
- Real-time spectral analysis and visualization
- A/B comparison tools
- Preset morphing
- Expanded saturation models
- Automated gain matching
- Plugin preset browser and tagging system

---

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

---

## Documentation

For deeper technical documentation, see:

```text
/docs/ARCHITECTURE.md
```

---

**Built with professional audio engineering principles for high-quality, real-time DSP workflows.**
