# Phantom-Mix-Suite

**Professional Audio DSP Plugin Suite** — High-fidelity mixing and mastering processors with analog-modeled saturation and linear-phase processing.

## Overview

Phantom-Mix-Suite is a collection of professional-grade audio plugins engineered for mixing and mastering workflows. Built with C++ and JUCE, the suite delivers transparent processing with authentic analog characteristics through physics-based modeling of tube saturation, transformer nonlinearity, and filter topologies.

### Key Features

- **Linear-Phase EQ** — Parametric and dynamic equalization with zero phase distortion
- - **Analog-Modeled Saturation** — Tube and transformer saturation with harmonic distortion modeling
  - - **Multi-Band Dynamics** — Frequency-selective compression and expansion
    - - **Anti-Aliasing Architecture** — Oversampling and brick-wall filtering for aliasing prevention
      - - **High-Resolution Support** — 32-bit float and 64-bit double precision processing
        - - **CPU Optimization** — SIMD vectorization and efficient DSP algorithms
          - - **Cross-Platform** — AU, VST3, and AAX support on macOS, Windows, and Linux
           
            - ## Architecture
           
            - The plugin suite is organized around core DSP primitives that handle signal processing with precision and efficiency:
           
            - ### Parametric EQ
           
            - Implements 5-band peaking and shelving EQ with:
            - - Bi-quad filter topology for stable cascade processing
              - - Frequency-dependent gain compensation
                - - RMS metering on input/output
                  - - Real-time coefficient updates without clicks
                   
                    - ```cpp
                      class ParametricEQ {
                          static constexpr int NUM_BANDS = 5;
                          std::array<FilterBand, NUM_BANDS> bands;

                          void processBlock(juce::AudioBuffer<float>& buffer) {
                              for (auto& band : bands) {
                                  for (int s = 0; s < buffer.getNumSamples(); ++s) {
                                      sample = band.processSample(sample);
                                  }
                              }
                          }
                      };
                      ```

                      ### Filter Architecture

                      Each filter band maintains state for Direct Form II implementation:
                      - Numerator coefficients: b0, b1, b2
                      - - Denominator coefficients: a0, a1, a2
                        - - State variables: x1, x2, y1, y2
                          - - Real-time coefficient calculation from user parameters
                           
                            - ```cpp
                              struct FilterBand {
                                  double b0, b1, b2, a0, a1, a2;
                                  double x1 = 0.0, x2 = 0.0;
                                  double y1 = 0.0, y2 = 0.0;

                                  inline double processSample(double input) {
                                      double output = b0*input + b1*x1 + b2*x2 - a1*y1 - a2*y2;
                                      x2 = x1; x1 = input;
                                      y2 = y1; y1 = output;
                                      return output;
                                  }
                              };
                              ```

                              ### Anti-Aliasing Strategy

                              - **Oversampling**: 2x-4x configurable for saturation processing
                              - - **Low-Pass Filtering**: Brick-wall filters at Nyquist
                                - - **Fade-Out Dither**: Noise shaping for dithering
                                  - - **Latency Compensation**: Zero-latency mode for low-latency recording
                                   
                                    - ## Code Samples
                                   
                                    - ### EQ Parameter Mapping
                                   
                                    - ```cpp
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

                                      ### Real-Time Processing Loop

                                      ```cpp
                                      void ParametricEQ::processBlock(juce::AudioBuffer<float>& buffer) {
                                          int numSamples = buffer.getNumSamples();
                                          int numChannels = buffer.getNumChannels();

                                          for (int ch = 0; ch < numChannels; ++ch) {
                                              auto* data = buffer.getWritePointer(ch);
                                              for (int s = 0; s < numSamples; ++s) {
                                                  float sample = data[s];
                                                  for (auto& band : bands) {
                                                      sample = band.processSample(sample);
                                                  }
                                                  data[s] = sample;
                                              }
                                          }
                                      }
                                      ```

                                      ## DSP Techniques

                                      ### Bi-Quad Filters

                                      Each peaking or shelving band is implemented as a second-order recursive filter (bi-quad):
                                      - Numerator: b0, b1, b2 feedforward taps
                                      - - Denominator: a1, a2 feedback taps (normalized)
                                        - - State: Two samples of history for both input and output
                                         
                                          - This topology is numerically stable and efficient, requiring only 6 multiplies and 5 adds per sample.
                                         
                                          - ### Coefficient Calculation
                                         
                                          - Band parameters (frequency, gain, Q) are converted to filter coefficients using standard audio EQ formulas:
                                          - - Bandwidth calculation: BW = f / Q
                                            - - Omega (angular frequency): ω = 2π * f / sample_rate
                                              - - Gain factor: A = 10^(dB/40)
                                               
                                                - ### Cascading Filters
                                               
                                                - Multiple bands are cascaded to create the overall EQ response:
                                                - - Bands are applied serially in frequency order
                                                  - - Each band preserves output state for the next band
                                                    - - Gain makeup is applied at the output to compensate for total gain change
                                                     
                                                      - ## Building & Integration
                                                     
                                                      - ### CMake Configuration
                                                     
                                                      - ```bash
                                                        cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
                                                        cmake --build build --parallel
                                                        ```

                                                        ### JUCE Processor

                                                        Plugins inherit from `juce::AudioProcessor` and implement:
                                                        - `processBlock()` — Core audio processing callback
                                                        - - `getParameters()` — Parameter list for UI/automation
                                                          - - `prepareToPlay()` — Initialization with sample rate
                                                            - - `releaseResources()` — Cleanup
                                                             
                                                              - ## Performance Characteristics
                                                             
                                                              - - **CPU Usage**: ~2-5% per instance (stereo, 48kHz)
                                                                - - **Latency**: 0 samples (linear-phase via phase correction)
                                                                  - - **Memory**: ~512KB per instance
                                                                    - - **Precision**: 64-bit double processing, 32-bit output
                                                                     
                                                                      - ## Design Philosophy
                                                                     
                                                                      - Phantom-Mix-Suite prioritizes:
                                                                      - 1. **Audio Quality** — Transparent processing with zero artifacts
                                                                        2. 2. **CPU Efficiency** — Optimized DSP algorithms and cache locality
                                                                           3. 3. **User Experience** — Intuitive parameters and real-time feedback
                                                                              4. 4. **Flexibility** — Modular design for custom processing chains
                                                                                 5. 5. **Portability** — Cross-platform support via JUCE framework
                                                                                   
                                                                                    6. ## Use Cases
                                                                                   
                                                                                    7. - **Mixing** — Surgical EQ and dynamic control for individual tracks
                                                                                       - - **Mastering** — Linear-phase processing and loudness management
                                                                                         - - **Sound Design** — Saturation and harmonic coloration for creative effects
                                                                                           - - **Live Sound** — Real-time processing with zero-latency mode
                                                                                             - - **Post-Production** — Restoration and enhancement of recorded material
                                                                                              
                                                                                               - ## Implementation Notes
                                                                                              
                                                                                               - - Filter state is maintained per-channel to prevent cross-channel artifacts
                                                                                                 - - All DSP operations use double-precision internally for numerical stability
                                                                                                   - - Parameter changes are applied at block boundaries to prevent clicks
                                                                                                     - - Metering is updated once per block for UI responsiveness
                                                                                                      
                                                                                                       - ## Future Enhancements
                                                                                                      
                                                                                                       - - GPU-accelerated processing for multi-band operations
                                                                                                         - - Machine learning-based mastering suggestions
                                                                                                           - - Sidechain filtering for dynamics and EQ
                                                                                                             - - Spectral analysis and real-time visualization
                                                                                                               - - A/B comparison and preset morphing
                                                                                                                
                                                                                                                 - ## License
                                                                                                                
                                                                                                                 - MIT License — See LICENSE file for details.
                                                                                                                
                                                                                                                 - ---
                                                                                                                 
                                                                                                                 **Built with professional audio engineering principles.** For detailed architecture documentation, see `/docs/ARCHITECTURE.md`.
