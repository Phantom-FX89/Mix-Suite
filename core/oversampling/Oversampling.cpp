#include "core/oversampling/Oversampling.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace phantom::core {

namespace {

constexpr std::size_t kHalfBandTaps = 31;

[[nodiscard]] double sinc(double value) noexcept {
    if (std::abs(value) < 1.0e-12) {
        return 1.0;
    }
    const auto argument = std::numbers::pi * value;
    return std::sin(argument) / argument;
}

[[nodiscard]] std::vector<double> designHalfBandFir(std::size_t taps) {
    std::vector<double> coefficients(taps, 0.0);
    const auto centre = static_cast<double>(taps - 1) / 2.0;
    constexpr double cutoff = 0.23;

    for (std::size_t index = 0; index < taps; ++index) {
        const auto offset = static_cast<double>(index) - centre;
        const auto ideal = 2.0 * cutoff * sinc(2.0 * cutoff * offset);
        const auto window = 0.54 - (0.46 * std::cos((2.0 * std::numbers::pi * static_cast<double>(index)) / static_cast<double>(taps - 1)));
        coefficients[index] = ideal * window;
    }

    const auto centreIndex = taps / 2;
    for (std::size_t index = 0; index < taps; ++index) {
        if ((index != centreIndex) && ((index % 2) == (centreIndex % 2))) {
            coefficients[index] = 0.0;
        }
    }
    coefficients[centreIndex] = 0.5;

    auto sum = 0.0;
    for (const auto coefficient : coefficients) {
        sum += coefficient;
    }
    for (auto& coefficient : coefficients) {
        coefficient /= sum;
    }

    return coefficients;
}

}  // namespace

OversamplingProcessor::HalfBandStage::HalfBandStage()
    : fullCoefficients_(designHalfBandFir(kHalfBandTaps)) {
    for (std::size_t index = 0; index < fullCoefficients_.size(); ++index) {
        if ((index % 2) == 0) {
            evenCoefficients_.push_back(fullCoefficients_[index]);
        } else {
            oddCoefficients_.push_back(fullCoefficients_[index]);
        }
    }
}

void OversamplingProcessor::HalfBandStage::prepare(std::size_t numChannels) {
    numChannels_ = numChannels;
    const auto historyLength = std::max(evenCoefficients_.size(), oddCoefficients_.size());
    upHistory_.assign(numChannels_, std::vector<double>(historyLength, 0.0));
    downHistory_.assign(numChannels_, std::vector<double>(fullCoefficients_.size(), 0.0));
    downParity_.assign(numChannels_, 0);
    upIndices_.assign(numChannels_, 0);
    downIndices_.assign(numChannels_, 0);
}

void OversamplingProcessor::HalfBandStage::reset() {
    for (auto& channelHistory : upHistory_) {
        std::fill(channelHistory.begin(), channelHistory.end(), 0.0);
    }
    for (auto& channelHistory : downHistory_) {
        std::fill(channelHistory.begin(), channelHistory.end(), 0.0);
    }
    std::fill(downParity_.begin(), downParity_.end(), 0);
    std::fill(upIndices_.begin(), upIndices_.end(), 0);
    std::fill(downIndices_.begin(), downIndices_.end(), 0);
}

std::size_t OversamplingProcessor::HalfBandStage::inputRateLatencySamples() const noexcept {
    return (fullCoefficients_.size() - 1) / 4;
}

void OversamplingProcessor::HalfBandStage::upsample(const AudioBlockView<const double>& input, const AudioBlockView<double>& output) {
    if (output.numSamples() != (input.numSamples() * 2U)) {
        throw std::invalid_argument("HalfBandStage upsample size mismatch");
    }

    for (std::size_t channel = 0; channel < numChannels_; ++channel) {
        auto inputChannel = input.channel(channel);
        auto outputChannel = output.channel(channel);

        for (std::size_t sampleIndex = 0; sampleIndex < inputChannel.size(); ++sampleIndex) {
            pushUpSample(channel, inputChannel[sampleIndex]);
            outputChannel[(sampleIndex * 2U)] = 2.0 * dotPhase(upHistory_[channel], upIndices_[channel], evenCoefficients_);
            outputChannel[(sampleIndex * 2U) + 1U] = 2.0 * dotPhase(upHistory_[channel], upIndices_[channel], oddCoefficients_);
        }
    }
}

void OversamplingProcessor::HalfBandStage::downsample(const AudioBlockView<const double>& input, const AudioBlockView<double>& output) {
    if (input.numSamples() != (output.numSamples() * 2U)) {
        throw std::invalid_argument("HalfBandStage downsample size mismatch");
    }

    for (std::size_t channel = 0; channel < numChannels_; ++channel) {
        auto inputChannel = input.channel(channel);
        auto outputChannel = output.channel(channel);
        std::size_t outputIndex = 0;

        for (const auto sample : inputChannel) {
            pushDownSample(channel, sample);
            if ((downParity_[channel] % 2U) == 0U) {
                outputChannel[outputIndex++] = dotPhase(downHistory_[channel], downIndices_[channel], fullCoefficients_);
            }
            ++downParity_[channel];
        }
    }
}

double OversamplingProcessor::HalfBandStage::dotPhase(const std::vector<double>& history,
                                                      std::size_t writeIndex,
                                                      std::span<const double> coefficients) {
    auto accumulator = 0.0;
    auto historyIndex = writeIndex;

    for (const auto coefficient : coefficients) {
        accumulator += coefficient * history[historyIndex];
        historyIndex = (historyIndex == 0U) ? (history.size() - 1U) : (historyIndex - 1U);
    }

    return sanitizeSample(accumulator);
}

void OversamplingProcessor::HalfBandStage::pushUpSample(std::size_t channel, double sample) {
    auto& history = upHistory_[channel];
    auto& index = upIndices_[channel];
    index = (index + 1U) % history.size();
    history[index] = sanitizeSample(sample);
}

void OversamplingProcessor::HalfBandStage::pushDownSample(std::size_t channel, double sample) {
    auto& history = downHistory_[channel];
    auto& index = downIndices_[channel];
    index = (index + 1U) % history.size();
    history[index] = sanitizeSample(sample);
}

void OversamplingProcessor::prepare(double sampleRate, std::size_t maxBlockSize, std::size_t numChannels) {
    sampleRate_ = sampleRate;
    maxBlockSize_ = maxBlockSize;
    numChannels_ = numChannels;

    if (stage2x_ == nullptr) {
        stage2x_ = std::make_unique<HalfBandStage>();
    }
    if (stage4x_ == nullptr) {
        stage4x_ = std::make_unique<HalfBandStage>();
    }

    stage2x_->prepare(numChannels_);
    stage4x_->prepare(numChannels_);
    ensureScratchCapacity(oversamplingFactor(mode_));
    reset();
}

void OversamplingProcessor::reset() {
    if (stage2x_ != nullptr) {
        stage2x_->reset();
    }
    if (stage4x_ != nullptr) {
        stage4x_->reset();
    }
    stage2xBlock_.clear();
    stage4xBlock_.clear();
    stage4xProcessedBlock_.clear();
    stage2xDownBlock_.clear();
}

void OversamplingProcessor::setMode(OversamplingMode mode) {
    if (mode == OversamplingMode::x8HQ) {
        throw std::invalid_argument("x8HQ is reserved for a later quality tier");
    }
    mode_ = mode;
    ensureScratchCapacity(oversamplingFactor(mode_));
}

OversamplingMode OversamplingProcessor::mode() const noexcept {
    return mode_;
}

std::size_t OversamplingProcessor::getLatencySamples() const noexcept {
    const auto factor = oversamplingFactor(mode_);
    if (factor <= 1U || stage2x_ == nullptr || stage4x_ == nullptr) {
        return 0;
    }

    auto latency = stage2x_->inputRateLatencySamples();
    if (factor == 4U) {
        latency += (stage4x_->inputRateLatencySamples() / 2U);
    }
    return latency;
}

OversampledProcessorContext OversamplingProcessor::currentContext(std::size_t blockSize) const noexcept {
    return {
        .sampleRate = sampleRate_ * static_cast<double>(oversamplingFactor(mode_)),
        .mode = mode_,
        .oversamplingFactor = oversamplingFactor(mode_),
        .blockSize = blockSize * oversamplingFactor(mode_)
    };
}

void OversamplingProcessor::ensureScratchCapacity(std::size_t factor) {
    if (numChannels_ == 0U || maxBlockSize_ == 0U) {
        return;
    }

    const auto samples2x = maxBlockSize_ * 2U;
    const auto samples4x = maxBlockSize_ * factor;
    stage2xBlock_.resize(numChannels_, samples2x);
    stage4xBlock_.resize(numChannels_, samples4x);
    stage4xProcessedBlock_.resize(numChannels_, samples4x);
    stage2xDownBlock_.resize(numChannels_, samples2x);
}

}  // namespace phantom::core
