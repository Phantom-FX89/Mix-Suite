#pragma once

#include "core/processing/Types.h"

namespace phantom::core {

struct MeterBallistics {
    double attackSeconds {0.01};
    double releaseSeconds {0.2};
};

class LevelMeter {
public:
    void prepare(double sampleRate);
    void reset(double initialLinearValue = 0.0);
    void setBallistics(MeterBallistics ballistics);

    [[nodiscard]] double processSample(double levelLinear);
    [[nodiscard]] double linearValue() const noexcept;
    [[nodiscard]] double dbValue() const noexcept;

private:
    double sampleRate_ {48000.0};
    double attackCoefficient_ {0.0};
    double releaseCoefficient_ {0.0};
    double value_ {0.0};
};

class GainReductionMeter {
public:
    void prepare(double sampleRate);
    void reset(double initialGainReductionDb = 0.0);
    void setBallistics(MeterBallistics ballistics);

    [[nodiscard]] double processSample(double gainReductionDb);
    [[nodiscard]] double gainReductionDb() const noexcept;

private:
    double sampleRate_ {48000.0};
    double attackCoefficient_ {0.0};
    double releaseCoefficient_ {0.0};
    double gainReductionDb_ {0.0};
};

}  // namespace phantom::core
