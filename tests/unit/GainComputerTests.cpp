#include "tests/TestRunner.h"

#include "dsp/gain/GainComputer.h"

namespace phantom::testing {

void runGainComputerTests() {
    using namespace phantom::dsp;

    GainComputer gainComputer;
    gainComputer.prepare(48000.0);
    gainComputer.setCurve({
        .thresholdDb = -18.0,
        .ratio = 4.0,
        .kneeMode = GainComputerKneeMode::hard,
        .kneeWidthDb = 0.0,
        .maxGainReductionDb = 60.0
    });
    gainComputer.setTiming(0.001, 0.050);

    PHANTOM_EXPECT_NEAR(gainComputer.evaluateTargetGainReduction(-6.0), 9.0, 1.0e-9);

    gainComputer.setCurve({
        .thresholdDb = -18.0,
        .ratio = 4.0,
        .kneeMode = GainComputerKneeMode::soft,
        .kneeWidthDb = 6.0,
        .maxGainReductionDb = 60.0
    });
    const auto softKneeReduction = gainComputer.evaluateTargetGainReduction(-18.0);
    PHANTOM_EXPECT_TRUE(softKneeReduction > 0.0);
    PHANTOM_EXPECT_TRUE(softKneeReduction < 9.0);

    gainComputer.setCurve({
        .thresholdDb = -18.0,
        .ratio = 8.0,
        .kneeMode = GainComputerKneeMode::hard,
        .kneeWidthDb = 0.0,
        .maxGainReductionDb = 60.0,
        .manualMakeupGainDb = 2.0,
        .autoMakeupEnabled = true
    });
    gainComputer.setAutoMakeupStrategy([](const GainComputerCurve& curve) {
        return (curve.thresholdDb * -0.25) / curve.ratio;
    });

    GainComputerResult result {};
    for (int index = 0; index < 64; ++index) {
        result = gainComputer.processLevel(-3.0);
    }

    PHANTOM_EXPECT_TRUE(result.appliedGainReductionDb > 0.0);
    PHANTOM_EXPECT_TRUE(result.linearGain < 1.0);
    PHANTOM_EXPECT_TRUE(result.gainReductionNormalized > 0.0);
}

}  // namespace phantom::testing
