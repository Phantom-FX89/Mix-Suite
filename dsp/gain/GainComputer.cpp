#include "dsp/gain/GainComputer.h"

#include <algorithm>
#include <cmath>

namespace phantom::dsp {

namespace {

[[nodiscard]] double softKneeGainReduction(double inputLevelDb, const GainComputerCurve& curve) {
    const auto halfKnee = curve.kneeWidthDb * 0.5;
    const auto lowerBound = curve.thresholdDb - halfKnee;
    const auto upperBound = curve.thresholdDb + halfKnee;

    if (inputLevelDb <= lowerBound) {
        return 0.0;
    }
    if (inputLevelDb >= upperBound) {
        return std::max(0.0, (1.0 - (1.0 / curve.ratio)) * (inputLevelDb - curve.thresholdDb));
    }

    const auto distanceIntoKnee = inputLevelDb - lowerBound;
    return std::max(0.0, (1.0 - (1.0 / curve.ratio)) * distanceIntoKnee * distanceIntoKnee / (2.0 * curve.kneeWidthDb));
}

}  // namespace

void GainComputer::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    setCurve(curve_);
    setTiming(0.001, 0.050);
    reset();
}

void GainComputer::reset(double initialGainReductionDb) {
    appliedGainReductionDb_ = std::max(0.0, initialGainReductionDb);
}

void GainComputer::setCurve(GainComputerCurve curve) {
    curve_ = curve;
    curve_.ratio = std::max(curve_.ratio, 1.0);
    curve_.kneeWidthDb = std::max(0.0, curve_.kneeWidthDb);
    curve_.maxGainReductionDb = std::max(0.0, curve_.maxGainReductionDb);
}

void GainComputer::setTiming(double attackSeconds, double releaseSeconds) {
    attackCoefficient_ = core::timeToCoefficient(attackSeconds, sampleRate_);
    releaseCoefficient_ = core::timeToCoefficient(releaseSeconds, sampleRate_);
}

void GainComputer::setAutoMakeupStrategy(std::function<double(const GainComputerCurve&)> callback) {
    autoMakeupStrategy_ = std::move(callback);
}

GainComputerResult GainComputer::processLevel(double inputLevelDb) {
    const auto targetGainReductionDb = evaluateTargetGainReduction(inputLevelDb);
    const auto coefficient = (targetGainReductionDb > appliedGainReductionDb_) ? attackCoefficient_ : releaseCoefficient_;
    appliedGainReductionDb_ = (coefficient * appliedGainReductionDb_) + ((1.0 - coefficient) * targetGainReductionDb);

    const auto totalOutputGainDb = makeupGainDb() - appliedGainReductionDb_;
    return {
        .inputLevelDb = inputLevelDb,
        .targetGainReductionDb = targetGainReductionDb,
        .appliedGainReductionDb = appliedGainReductionDb_,
        .linearGain = core::dbToLinear(totalOutputGainDb),
        .outputGainDb = totalOutputGainDb,
        .gainReductionNormalized = (curve_.maxGainReductionDb > 0.0) ? std::clamp(appliedGainReductionDb_ / curve_.maxGainReductionDb, 0.0, 1.0) : 0.0
    };
}

double GainComputer::evaluateTargetGainReduction(double inputLevelDb) const {
    return staticCurveGainReduction(inputLevelDb);
}

double GainComputer::appliedGainReductionDb() const noexcept {
    return appliedGainReductionDb_;
}

double GainComputer::attackCoefficient() const noexcept {
    return attackCoefficient_;
}

double GainComputer::releaseCoefficient() const noexcept {
    return releaseCoefficient_;
}

double GainComputer::staticCurveGainReduction(double inputLevelDb) const {
    double gainReductionDb = 0.0;

    switch (curve_.kneeMode) {
        case GainComputerKneeMode::hard:
            if (inputLevelDb > curve_.thresholdDb) {
                gainReductionDb = (1.0 - (1.0 / curve_.ratio)) * (inputLevelDb - curve_.thresholdDb);
            }
            break;
        case GainComputerKneeMode::soft:
            gainReductionDb = softKneeGainReduction(inputLevelDb, curve_);
            break;
        case GainComputerKneeMode::variable: {
            auto variableCurve = curve_;
            variableCurve.kneeWidthDb = std::max(1.0, curve_.kneeWidthDb);
            gainReductionDb = softKneeGainReduction(inputLevelDb, variableCurve);
            break;
        }
    }

    return std::clamp(gainReductionDb, 0.0, curve_.maxGainReductionDb);
}

double GainComputer::makeupGainDb() const {
    if (curve_.autoMakeupEnabled && autoMakeupStrategy_) {
        return autoMakeupStrategy_(curve_);
    }
    return curve_.manualMakeupGainDb;
}

}  // namespace phantom::dsp
