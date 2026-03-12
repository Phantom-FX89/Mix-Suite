#include "dsp/filters/Biquad.h"

#include "core/processing/Types.h"

#include <cmath>
#include <numbers>

namespace phantom::dsp {

namespace {

[[nodiscard]] BiquadCoefficients normalize(double b0, double b1, double b2, double a0, double a1, double a2) {
    return {
        .b0 = b0 / a0,
        .b1 = b1 / a0,
        .b2 = b2 / a0,
        .a1 = a1 / a0,
        .a2 = a2 / a0
    };
}

}  // namespace

void BiquadSection::reset() {
    x1_ = 0.0;
    x2_ = 0.0;
    y1_ = 0.0;
    y2_ = 0.0;
}

void BiquadSection::setCoefficients(BiquadCoefficients coefficients) {
    coefficients_ = coefficients;
}

double BiquadSection::processSample(double inputSample) {
    const auto output = (coefficients_.b0 * inputSample) + (coefficients_.b1 * x1_) + (coefficients_.b2 * x2_) -
                        (coefficients_.a1 * y1_) - (coefficients_.a2 * y2_);

    x2_ = x1_;
    x1_ = inputSample;
    y2_ = y1_;
    y1_ = core::sanitizeSample(output);
    return y1_;
}

BiquadCoefficients makeHighPass(double sampleRate, double frequencyHz, double q) {
    const auto omega = 2.0 * std::numbers::pi * (frequencyHz / sampleRate);
    const auto cosine = std::cos(omega);
    const auto sine = std::sin(omega);
    const auto alpha = sine / (2.0 * q);

    const auto b0 = (1.0 + cosine) / 2.0;
    const auto b1 = -(1.0 + cosine);
    const auto b2 = (1.0 + cosine) / 2.0;
    const auto a0 = 1.0 + alpha;
    const auto a1 = -2.0 * cosine;
    const auto a2 = 1.0 - alpha;

    return normalize(b0, b1, b2, a0, a1, a2);
}

BiquadCoefficients makeBell(double sampleRate, double frequencyHz, double q, double gainDb) {
    const auto amplitude = std::pow(10.0, gainDb / 40.0);
    const auto omega = 2.0 * std::numbers::pi * (frequencyHz / sampleRate);
    const auto cosine = std::cos(omega);
    const auto sine = std::sin(omega);
    const auto alpha = sine / (2.0 * q);

    const auto b0 = 1.0 + (alpha * amplitude);
    const auto b1 = -2.0 * cosine;
    const auto b2 = 1.0 - (alpha * amplitude);
    const auto a0 = 1.0 + (alpha / amplitude);
    const auto a1 = -2.0 * cosine;
    const auto a2 = 1.0 - (alpha / amplitude);

    return normalize(b0, b1, b2, a0, a1, a2);
}

BiquadCoefficients makeLowShelf(double sampleRate, double frequencyHz, double q, double gainDb) {
    const auto amplitude = std::pow(10.0, gainDb / 40.0);
    const auto omega = 2.0 * std::numbers::pi * (frequencyHz / sampleRate);
    const auto cosine = std::cos(omega);
    const auto sine = std::sin(omega);
    const auto beta = std::sqrt(amplitude) / q;

    const auto b0 = amplitude * ((amplitude + 1.0) - ((amplitude - 1.0) * cosine) + (beta * sine));
    const auto b1 = 2.0 * amplitude * ((amplitude - 1.0) - ((amplitude + 1.0) * cosine));
    const auto b2 = amplitude * ((amplitude + 1.0) - ((amplitude - 1.0) * cosine) - (beta * sine));
    const auto a0 = (amplitude + 1.0) + ((amplitude - 1.0) * cosine) + (beta * sine);
    const auto a1 = -2.0 * ((amplitude - 1.0) + ((amplitude + 1.0) * cosine));
    const auto a2 = (amplitude + 1.0) + ((amplitude - 1.0) * cosine) - (beta * sine);

    return normalize(b0, b1, b2, a0, a1, a2);
}

BiquadCoefficients makeHighShelf(double sampleRate, double frequencyHz, double q, double gainDb) {
    const auto amplitude = std::pow(10.0, gainDb / 40.0);
    const auto omega = 2.0 * std::numbers::pi * (frequencyHz / sampleRate);
    const auto cosine = std::cos(omega);
    const auto sine = std::sin(omega);
    const auto beta = std::sqrt(amplitude) / q;

    const auto b0 = amplitude * ((amplitude + 1.0) + ((amplitude - 1.0) * cosine) + (beta * sine));
    const auto b1 = -2.0 * amplitude * ((amplitude - 1.0) + ((amplitude + 1.0) * cosine));
    const auto b2 = amplitude * ((amplitude + 1.0) + ((amplitude - 1.0) * cosine) - (beta * sine));
    const auto a0 = (amplitude + 1.0) - ((amplitude - 1.0) * cosine) + (beta * sine);
    const auto a1 = 2.0 * ((amplitude - 1.0) - ((amplitude + 1.0) * cosine));
    const auto a2 = (amplitude + 1.0) - ((amplitude - 1.0) * cosine) - (beta * sine);

    return normalize(b0, b1, b2, a0, a1, a2);
}

}  // namespace phantom::dsp
