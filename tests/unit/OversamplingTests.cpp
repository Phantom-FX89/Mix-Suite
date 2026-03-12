#include "tests/TestRunner.h"

#include "core/audio/AudioBlock.h"
#include "core/oversampling/Oversampling.h"

#include <array>
#include <cmath>
#include <limits>
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
    oversampling.processBlock(input.constView(), output.view(), [&](const auto& inBlock, const auto& outBlock, const auto& context) {
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
    const auto latencySamples = oversampling.getLatencySamples();
    PHANTOM_EXPECT_TRUE(latencySamples > 0);

    auto bestRmse = std::numeric_limits<double>::max();
    for (std::size_t shift = 0; shift <= (latencySamples + 4U); ++shift) {
        auto errorSum = 0.0;
        auto count = 0.0;
        for (std::size_t index = 8; index + shift < input.numSamples() && index < 48; ++index) {
            const auto error = output.channel(0)[index + shift] - input.channel(0)[index];
            errorSum += error * error;
            count += 1.0;
        }
        bestRmse = std::min(bestRmse, std::sqrt(errorSum / count));
    }
    PHANTOM_EXPECT_NEAR(bestRmse, 0.0, 0.35);

    oversampling.setMode(OversamplingMode::x4);
    PHANTOM_EXPECT_TRUE(oversampling.getLatencySamples() >= 3);
}

}  // namespace phantom::testing
