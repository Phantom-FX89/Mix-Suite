#pragma once

#include "core/audio/AudioBlock.h"
#include "core/processing/Types.h"

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace phantom::core {

struct OversampledProcessorContext {
    double sampleRate {48000.0};
    OversamplingMode mode {OversamplingMode::x1};
    std::size_t oversamplingFactor {1};
    std::size_t blockSize {0};
};

struct OversamplingLatencyInfo {
    std::size_t inputRateLatencySamples {0};
    std::size_t oversampledDomainLatencySamples {0};
};

class OversamplingProcessor {
public:
    void prepare(double sampleRate, std::size_t maxBlockSize, std::size_t numChannels);
    void reset();
    void setMode(OversamplingMode mode);

    [[nodiscard]] OversamplingMode mode() const noexcept;
    [[nodiscard]] std::size_t getLatencySamples() const noexcept;
    [[nodiscard]] OversampledProcessorContext currentContext(std::size_t blockSize) const noexcept;

    template <typename Processor>
    void processBlock(const AudioBlockView<const double>& input, const AudioBlockView<double>& output, Processor&& processor);

private:
    class HalfBandStage {
    public:
        HalfBandStage();

        void prepare(std::size_t numChannels);
        void reset();
        [[nodiscard]] std::size_t inputRateLatencySamples() const noexcept;
        void upsample(const AudioBlockView<const double>& input, const AudioBlockView<double>& output);
        void downsample(const AudioBlockView<const double>& input, const AudioBlockView<double>& output);

    private:
        static double dotPhase(const std::vector<double>& history, std::size_t writeIndex, std::span<const double> coefficients);
        void pushUpSample(std::size_t channel, double sample);
        void pushDownSample(std::size_t channel, double sample);

        std::size_t numChannels_ {0};
        std::vector<double> fullCoefficients_;
        std::vector<double> evenCoefficients_;
        std::vector<double> oddCoefficients_;
        std::vector<std::vector<double>> upHistory_;
        std::vector<std::vector<double>> downHistory_;
        std::vector<std::size_t> downParity_;
        std::vector<std::size_t> upIndices_;
        std::vector<std::size_t> downIndices_;
    };

    void ensureScratchCapacity(std::size_t factor);

    double sampleRate_ {48000.0};
    std::size_t maxBlockSize_ {0};
    std::size_t numChannels_ {0};
    OversamplingMode mode_ {OversamplingMode::x1};
    std::unique_ptr<HalfBandStage> stage2x_;
    std::unique_ptr<HalfBandStage> stage4x_;
    OwnedAudioBlock<double> stage2xBlock_;
    OwnedAudioBlock<double> stage4xBlock_;
    OwnedAudioBlock<double> stage4xProcessedBlock_;
    OwnedAudioBlock<double> stage2xDownBlock_;
};

}  // namespace phantom::core

#include "core/oversampling/Oversampling.tpp"
