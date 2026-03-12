#pragma once

#include "core/processing/Types.h"

#include <functional>

namespace phantom::dsp {

enum class GainComputerKneeMode {
    hard,
    soft,
    variable
};

struct GainComputerCurve {
    double thresholdDb {0.0};
    double ratio {4.0};
    GainComputerKneeMode kneeMode {GainComputerKneeMode::hard};
    double kneeWidthDb {0.0};
    double maxGainReductionDb {48.0};
    double manualMakeupGainDb {0.0};
    bool autoMakeupEnabled {false};
};

struct GainComputerResult {
    double inputLevelDb {0.0};
    double targetGainReductionDb {0.0};
    double appliedGainReductionDb {0.0};
    double linearGain {1.0};
    double outputGainDb {0.0};
    double gainReductionNormalized {0.0};
};

class GainComputer {
public:
    void prepare(double sampleRate);
    void reset(double initialGainReductionDb = 0.0);
    void setCurve(GainComputerCurve curve);
    void setTiming(double attackSeconds, double releaseSeconds);
    void setAutoMakeupStrategy(std::function<double(const GainComputerCurve&)> callback);

    [[nodiscard]] GainComputerResult processLevel(double inputLevelDb);
    [[nodiscard]] double evaluateTargetGainReduction(double inputLevelDb) const;
    [[nodiscard]] double appliedGainReductionDb() const noexcept;
    [[nodiscard]] double attackCoefficient() const noexcept;
    [[nodiscard]] double releaseCoefficient() const noexcept;

private:
    [[nodiscard]] double staticCurveGainReduction(double inputLevelDb) const;
    [[nodiscard]] double makeupGainDb() const;

    double sampleRate_ {48000.0};
    GainComputerCurve curve_ {};
    double attackCoefficient_ {0.0};
    double releaseCoefficient_ {0.0};
    double appliedGainReductionDb_ {0.0};
    std::function<double(const GainComputerCurve&)> autoMakeupStrategy_;
};

}  // namespace phantom::dsp
