#include "core/metering/Metering.h"

#include <algorithm>

namespace phantom::core {

void LevelMeter::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    setBallistics({});
    reset();
}

void LevelMeter::reset(double initialLinearValue) {
    value_ = sanitizeSample(initialLinearValue);
}

void LevelMeter::setBallistics(MeterBallistics ballistics) {
    attackCoefficient_ = timeToCoefficient(ballistics.attackSeconds, sampleRate_);
    releaseCoefficient_ = timeToCoefficient(ballistics.releaseSeconds, sampleRate_);
}

double LevelMeter::processSample(double levelLinear) {
    const auto rectified = std::abs(levelLinear);
    const auto coefficient = (rectified > value_) ? attackCoefficient_ : releaseCoefficient_;
    value_ = sanitizeSample((coefficient * value_) + ((1.0 - coefficient) * rectified));
    return value_;
}

double LevelMeter::linearValue() const noexcept {
    return value_;
}

double LevelMeter::dbValue() const noexcept {
    return linearToDb(value_);
}

void GainReductionMeter::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    setBallistics({.attackSeconds = 0.003, .releaseSeconds = 0.1});
    reset();
}

void GainReductionMeter::reset(double initialGainReductionDb) {
    gainReductionDb_ = std::max(0.0, initialGainReductionDb);
}

void GainReductionMeter::setBallistics(MeterBallistics ballistics) {
    attackCoefficient_ = timeToCoefficient(ballistics.attackSeconds, sampleRate_);
    releaseCoefficient_ = timeToCoefficient(ballistics.releaseSeconds, sampleRate_);
}

double GainReductionMeter::processSample(double gainReductionDb) {
    const auto clamped = std::max(0.0, gainReductionDb);
    const auto coefficient = (clamped > gainReductionDb_) ? attackCoefficient_ : releaseCoefficient_;
    gainReductionDb_ = sanitizeSample((coefficient * gainReductionDb_) + ((1.0 - coefficient) * clamped));
    return gainReductionDb_;
}

double GainReductionMeter::gainReductionDb() const noexcept {
    return gainReductionDb_;
}

}  // namespace phantom::core
