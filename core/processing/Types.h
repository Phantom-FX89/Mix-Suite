#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace phantom::core {

constexpr double kSilenceFloorDb = -160.0;
constexpr double kSilenceFloorLinear = 1.0e-8;
constexpr double kMinimumTimeSeconds = 1.0e-6;

enum class ChannelMode {
    mono,
    stereoLinked,
    stereoUnlinked,
    dualMono
};

enum class OversamplingMode {
    x1,
    x2,
    x4,
    x8HQ
};

struct PrepareSpec {
    double sampleRate {48000.0};
    std::size_t maxBlockSize {0};
    std::size_t numChannels {1};
    ChannelMode channelMode {ChannelMode::mono};
};

[[nodiscard]] inline std::size_t oversamplingFactor(OversamplingMode mode) noexcept {
    switch (mode) {
        case OversamplingMode::x1:
            return 1;
        case OversamplingMode::x2:
            return 2;
        case OversamplingMode::x4:
            return 4;
        case OversamplingMode::x8HQ:
            return 8;
    }
    return 1;
}

[[nodiscard]] inline double clampSilenceFloorLinear(double value) noexcept {
    return std::max(std::abs(value), kSilenceFloorLinear);
}

[[nodiscard]] inline double linearToDb(double value) noexcept {
    return 20.0 * std::log10(clampSilenceFloorLinear(value));
}

[[nodiscard]] inline double dbToLinear(double valueDb) noexcept {
    return std::pow(10.0, valueDb / 20.0);
}

[[nodiscard]] inline double timeToCoefficient(double timeSeconds, double sampleRate) noexcept {
    const auto safeTime = std::max(timeSeconds, kMinimumTimeSeconds);
    const auto safeSampleRate = std::max(sampleRate, 1.0);
    return std::exp(-1.0 / (safeTime * safeSampleRate));
}

[[nodiscard]] inline double sanitizeSample(double value) noexcept {
    if (!std::isfinite(value)) {
        return 0.0;
    }
    if (std::abs(value) < 1.0e-18) {
        return 0.0;
    }
    return value;
}

}  // namespace phantom::core
