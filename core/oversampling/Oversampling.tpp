#pragma once

#include <stdexcept>

namespace phantom::core {

template <typename Processor>
void OversamplingProcessor::processBlock(const AudioBlockView<const double>& input,
                                         const AudioBlockView<double>& output,
                                         Processor&& processor) {
    if (input.numChannels() != numChannels_ || output.numChannels() != numChannels_) {
        throw std::invalid_argument("OversamplingProcessor channel mismatch");
    }
    if (input.numSamples() != output.numSamples()) {
        throw std::invalid_argument("OversamplingProcessor block-size mismatch");
    }

    const auto blockSize = input.numSamples();
    const auto factor = oversamplingFactor(mode_);

    if (factor == 1) {
        std::forward<Processor>(processor)(input, output, currentContext(blockSize));
        return;
    }

    ensureScratchCapacity(factor);
    stage2x_->upsample(input, stage2xBlock_.view());

    if (factor == 2) {
        std::forward<Processor>(processor)(stage2xBlock_.view(), stage4xProcessedBlock_.view(), currentContext(blockSize));
        stage2x_->downsample(stage4xProcessedBlock_.constView(), output);
        return;
    }

    stage4x_->upsample(stage2xBlock_.constView(), stage4xBlock_.view());
    std::forward<Processor>(processor)(stage4xBlock_.view(), stage4xProcessedBlock_.view(), currentContext(blockSize));
    stage4x_->downsample(stage4xProcessedBlock_.constView(), stage2xDownBlock_.view());
    stage2x_->downsample(stage2xDownBlock_.constView(), output);
}

}  // namespace phantom::core
