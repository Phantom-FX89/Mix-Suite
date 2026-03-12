#include "tests/TestRunner.h"

#include "core/audio/AudioBlock.h"
#include "core/oversampling/Oversampling.h"

#include <array>
#include <cmath>
#include <numbers>

namespace phantom::testing {

void runOversamplingTests() {
    using namespace phantom::core;

    OwnedAudioBlock<double> input(1, 64);
    OwnedAudioBlock<double> output(1, 64);

    for (std::size_t index = 0; index < input.numSamples(); ++index) {
        input.channel(0)[index] = std::sin((2.0 * std::numbers::pi * 1000.0 * static_cast<double>(index)) / 48000.0);
    }

    OversamplingProcessor oversampling;
    oversampling.prepare(48000.0, 64, 1);
    oversampling.setMode(OversamplingMode::x2);

    double observedSampleRate = 0.0;
    oversampling.processBlock(input.view(), output.view(), [&](const auto& inBlock, const auto& outBlock, const auto& context) {
        observedSampleRate = context.sampleRate;
        for (std::size_t channel = 0; channel < inBlock.numChannels(); ++channel) {
            auto source = inBlock.channel(channel);
            auto destination = outBlock.channel(channel);
            for (std::size_t sample = 0; sample < source.size(); ++sample) {
                destination[sample] = source[sample];
            }
        }
    });

    PHANTOM_EXPECT_NEAR(observedSampleRate, 96000.0, 1.0e-9);
    PHANTOM_EXPECT_TRUE(oversampling.getLatencySamples() > 0);

    const auto errorAtLowRate = std::abs(output.channel(0)[8] - input.channel(0)[8]);
    PHANTOM_EXPECT_TRUE(errorAtLowRate < 0.15);

    oversampling.setMode(OversamplingMode::x4);
    PHANTOM_EXPECT_TRUE(oversampling.getLatencySamples() >= 3);
}

}  // namespace phantom::testing
