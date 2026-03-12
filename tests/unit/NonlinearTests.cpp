#include "tests/TestRunner.h"

#include "core/calibration/Calibration.h"
#include "dsp/nonlinear/Nonlinear.h"

namespace phantom::testing {

void runNonlinearTests() {
    using namespace phantom;

    core::CalibrationSystem calibration;
    calibration.registerPluginProfile({
        .pluginId = "nonlinear-test",
        .nominalOffsetFromReferenceDb = 0.0,
        .nonlinearOnsetOffsetDb = 6.0,
        .softSatOffsetDb = 10.0,
        .clipOffsetDb = 18.0
    });
    const auto profile = calibration.resolveProfile("nonlinear-test");
    PHANTOM_EXPECT_TRUE(profile.has_value());

    dsp::FetStage fet;
    fet.prepare(192000.0);
    dsp::StageTelemetry lightTelemetry {};
    dsp::StageTelemetry heavyTelemetry {};

    const auto lightOutput = fet.processSample(0.55,
                                               *profile,
                                               {
                                                   .inputLevelDbfs = -16.0,
                                                   .normalizedDrive = 0.25,
                                                   .gainReductionDb = 2.0,
                                                   .gainReductionNormalized = 0.15,
                                                   .effectiveSampleRate = 192000.0,
                                                   .oversamplingFactor = 4
                                               },
                                               &lightTelemetry);
    const auto heavyOutput = fet.processSample(0.55,
                                               *profile,
                                               {
                                                   .inputLevelDbfs = -8.0,
                                                   .normalizedDrive = 0.9,
                                                   .gainReductionDb = 10.0,
                                                   .gainReductionNormalized = 0.75,
                                                   .effectiveSampleRate = 192000.0,
                                                   .oversamplingFactor = 4
                                               },
                                               &heavyTelemetry);

    PHANTOM_EXPECT_TRUE(fet.preferredOversamplingMode() == core::OversamplingMode::x4);
    PHANTOM_EXPECT_TRUE(heavyTelemetry.harmonicActivityEstimate > lightTelemetry.harmonicActivityEstimate);
    PHANTOM_EXPECT_TRUE(std::abs(heavyOutput) < std::abs(lightOutput));

    dsp::TransformerStage transformer;
    transformer.prepare(96000.0);
    PHANTOM_EXPECT_TRUE(transformer.preferredOversamplingMode() == core::OversamplingMode::x2);
}

}  // namespace phantom::testing
