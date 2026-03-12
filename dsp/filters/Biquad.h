#pragma once

#include <array>

namespace phantom::dsp {

struct BiquadCoefficients {
    double b0 {1.0};
    double b1 {0.0};
    double b2 {0.0};
    double a1 {0.0};
    double a2 {0.0};
};

class BiquadSection {
public:
    void reset();
    void setCoefficients(BiquadCoefficients coefficients);
    [[nodiscard]] double processSample(double inputSample);

private:
    BiquadCoefficients coefficients_ {};
    double x1_ {0.0};
    double x2_ {0.0};
    double y1_ {0.0};
    double y2_ {0.0};
};

[[nodiscard]] BiquadCoefficients makeHighPass(double sampleRate, double frequencyHz, double q = 0.70710678118);
[[nodiscard]] BiquadCoefficients makeBell(double sampleRate, double frequencyHz, double q, double gainDb);
[[nodiscard]] BiquadCoefficients makeLowShelf(double sampleRate, double frequencyHz, double q, double gainDb);
[[nodiscard]] BiquadCoefficients makeHighShelf(double sampleRate, double frequencyHz, double q, double gainDb);

}  // namespace phantom::dsp
