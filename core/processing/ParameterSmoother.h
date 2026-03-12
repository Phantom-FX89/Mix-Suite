#pragma once

#include <algorithm>
#include <cmath>

namespace phantom::core {

class ParameterSmoother {
public:
    void reset(double sampleRate, double timeSeconds, double initialValue) {
        sampleRate_ = sampleRate;
        currentValue_ = initialValue;
        targetValue_ = initialValue;
        setTimeConstant(timeSeconds);
    }

    void setTimeConstant(double timeSeconds) {
        constexpr double kMinimumTimeSeconds = 1.0e-6;
        const auto safeTime = std::max(timeSeconds, kMinimumTimeSeconds);
        coefficient_ = std::exp(-1.0 / (safeTime * std::max(sampleRate_, 1.0)));
    }

    void setCurrentAndTargetValue(double value) {
        currentValue_ = value;
        targetValue_ = value;
    }

    void setTargetValue(double targetValue) {
        targetValue_ = targetValue;
    }

    [[nodiscard]] double currentValue() const noexcept { return currentValue_; }
    [[nodiscard]] double targetValue() const noexcept { return targetValue_; }

    [[nodiscard]] double getNextValue() {
        currentValue_ = (coefficient_ * currentValue_) + ((1.0 - coefficient_) * targetValue_);
        return currentValue_;
    }

    [[nodiscard]] double skip(std::size_t samples) {
        for (std::size_t index = 0; index < samples; ++index) {
            getNextValue();
        }
        return currentValue_;
    }

private:
    double sampleRate_ {48000.0};
    double coefficient_ {0.0};
    double currentValue_ {0.0};
    double targetValue_ {0.0};
};

}  // namespace phantom::core
