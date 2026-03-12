#include "dsp/analog/AnalogPrimitives.h"

#include <algorithm>
#include <cmath>

namespace phantom::dsp {

namespace {

[[nodiscard]] std::uint32_t fnv1a(std::string_view text, std::uint32_t seed) noexcept {
    auto hash = 2166136261u ^ seed;
    for (const auto character : text) {
        hash ^= static_cast<std::uint32_t>(character);
        hash *= 16777619u;
    }
    return hash;
}

[[nodiscard]] double normalizedRandom(std::uint32_t value) noexcept {
    return (static_cast<double>(value) / static_cast<double>(UINT32_MAX)) * 2.0 - 1.0;
}

}  // namespace

void ComponentVariance::setSeed(std::uint32_t seed) noexcept {
    seed_ = seed;
}

double ComponentVariance::multiplierFor(std::string_view componentId, double spreadFraction) const noexcept {
    const auto hash = fnv1a(componentId, seed_);
    return 1.0 + (normalizedRandom(hash) * std::max(0.0, spreadFraction));
}

void WhiteNoiseGenerator::setSeed(std::uint32_t seed) noexcept {
    state_ = seed;
}

void WhiteNoiseGenerator::setLevelDb(double levelDb) noexcept {
    levelLinear_ = core::dbToLinear(levelDb);
}

double WhiteNoiseGenerator::nextSample() noexcept {
    state_ ^= state_ << 13U;
    state_ ^= state_ >> 17U;
    state_ ^= state_ << 5U;
    return normalizedRandom(state_) * levelLinear_;
}

void StereoCrosstalk::setAmountDb(double amountDb) noexcept {
    coefficient_ = core::dbToLinear(amountDb);
}

std::array<double, 2> StereoCrosstalk::process(double left, double right) const noexcept {
    return {
        core::sanitizeSample(left + (right * coefficient_)),
        core::sanitizeSample(right + (left * coefficient_))
    };
}

void PowerSupplyMemory::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    attackCoefficient_ = core::timeToCoefficient(0.015, sampleRate_);
    releaseCoefficient_ = core::timeToCoefficient(0.220, sampleRate_);
    reset();
}

void PowerSupplyMemory::reset() {
    state_ = 0.0;
}

double PowerSupplyMemory::process(double normalizedDrive) {
    const auto target = std::clamp(normalizedDrive, 0.0, 2.0);
    const auto coefficient = (target > state_) ? attackCoefficient_ : releaseCoefficient_;
    state_ = core::sanitizeSample((coefficient * state_) + ((1.0 - coefficient) * target));
    return state_;
}

}  // namespace phantom::dsp
