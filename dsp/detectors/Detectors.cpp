#include "dsp/detectors/Detectors.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace phantom::dsp {

namespace {

constexpr std::size_t kMaxChannels = 2;

class SidechainFilter {
public:
    void prepare(double sampleRate) {
        sampleRate_ = sampleRate;
        rebuild();
    }

    void reset() {
        stageA_.reset();
        stageB_.reset();
    }

    void setConfig(SidechainFilterConfig config) {
        config_ = config;
        rebuild();
    }

    [[nodiscard]] double processSample(double inputSample) {
        if (config_.mode == SidechainFilterMode::bypass) {
            return inputSample;
        }

        auto sample = stageA_.processSample(inputSample);
        if (usesSecondStage_) {
            sample = stageB_.processSample(sample);
        }
        return sample;
    }

private:
    void rebuild() {
        stageA_.reset();
        stageB_.reset();
        usesSecondStage_ = false;

        switch (config_.mode) {
            case SidechainFilterMode::bypass:
                stageA_.setCoefficients({});
                break;
            case SidechainFilterMode::highPass:
                stageA_.setCoefficients(makeHighPass(sampleRate_, config_.frequencyHz, config_.q));
                break;
            case SidechainFilterMode::bell:
                stageA_.setCoefficients(makeBell(sampleRate_, config_.frequencyHz, config_.q, config_.gainDb));
                break;
            case SidechainFilterMode::tilt:
                stageA_.setCoefficients(makeLowShelf(sampleRate_, config_.frequencyHz, config_.q, -(config_.tiltDb * 0.5)));
                stageB_.setCoefficients(makeHighShelf(sampleRate_, config_.frequencyHz, config_.q, config_.tiltDb * 0.5));
                usesSecondStage_ = true;
                break;
        }
    }

    double sampleRate_ {48000.0};
    bool usesSecondStage_ {false};
    SidechainFilterConfig config_ {};
    BiquadSection stageA_ {};
    BiquadSection stageB_ {};
};

}  // namespace

class PeakDetector::SidechainFilterBank {
public:
    void prepare(double sampleRate) {
        for (auto& filter : filters_) {
            filter.prepare(sampleRate);
        }
    }

    void reset() {
        for (auto& filter : filters_) {
            filter.reset();
        }
    }

    void setConfig(SidechainFilterConfig config) {
        for (auto& filter : filters_) {
            filter.setConfig(config);
        }
    }

    [[nodiscard]] double process(std::size_t channelIndex, double sample) {
        return filters_.at(channelIndex).processSample(sample);
    }

private:
    std::array<SidechainFilter, kMaxChannels> filters_ {};
};

class RmsDetector::SidechainFilterBank {
public:
    void prepare(double sampleRate) {
        for (auto& filter : filters_) {
            filter.prepare(sampleRate);
        }
    }

    void reset() {
        for (auto& filter : filters_) {
            filter.reset();
        }
    }

    void setConfig(SidechainFilterConfig config) {
        for (auto& filter : filters_) {
            filter.setConfig(config);
        }
    }

    [[nodiscard]] double process(std::size_t channelIndex, double sample) {
        return filters_.at(channelIndex).processSample(sample);
    }

private:
    std::array<SidechainFilter, kMaxChannels> filters_ {};
};

void ProgramDependentDetectorScaffold::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    reset();
}

void ProgramDependentDetectorScaffold::reset() {
    state_ = {};
}

ProgramDependentDetectorState ProgramDependentDetectorScaffold::processSample(double levelLinear) {
    const auto input = std::clamp(levelLinear, 0.0, 4.0);
    const auto exposureCoefficient = core::timeToCoefficient(0.150, sampleRate_);
    const auto releaseFast = core::timeToCoefficient(0.080, sampleRate_);
    const auto releaseSlow = core::timeToCoefficient(0.450, sampleRate_);

    state_.exposure = (exposureCoefficient * state_.exposure) + ((1.0 - exposureCoefficient) * input);
    state_.releaseStageA = (releaseFast * state_.releaseStageA) + ((1.0 - releaseFast) * input);
    state_.releaseStageB = (releaseSlow * state_.releaseStageB) + ((1.0 - releaseSlow) * state_.releaseStageA);
    return state_;
}

const ProgramDependentDetectorState& ProgramDependentDetectorScaffold::state() const noexcept {
    return state_;
}

void PeakDetector::prepare(const core::PrepareSpec& spec) {
    spec_ = spec;
    if (filterBank_ == nullptr) {
        filterBank_ = new SidechainFilterBank();
    }
    filterBank_->prepare(spec.sampleRate);
    setTiming(timing_);
    reset();
}

void PeakDetector::reset(double initialLevel) {
    envelope_.fill(initialLevel);
    holdCounters_.fill(0U);
    if (filterBank_ != nullptr) {
        filterBank_->reset();
    }
}

void PeakDetector::setTiming(DetectorTiming timing) {
    timing_ = timing;
    attackCoefficient_ = core::timeToCoefficient(timing.attackSeconds, spec_.sampleRate);
    releaseCoefficient_ = core::timeToCoefficient(timing.releaseSeconds, spec_.sampleRate);
}

void PeakDetector::setSidechainConfig(SidechainFilterConfig config) {
    if (filterBank_ == nullptr) {
        return;
    }
    filterBank_->setConfig(config);
}

void PeakDetector::setChannelMode(core::ChannelMode channelMode) {
    spec_.channelMode = channelMode;
}

void PeakDetector::setLinkPolicy(std::optional<DetectorLinkPolicy> policy) {
    linkPolicy_ = policy;
}

void PeakDetector::setPeakEmphasis(bool enabled) {
    peakEmphasis_ = enabled;
}

void PeakDetector::setHoldSamples(std::size_t holdSamples) {
    holdSamples_ = holdSamples;
}

DetectorFrameOutput PeakDetector::processFrame(const DetectorFrameInput& input) {
    if ((spec_.channelMode == core::ChannelMode::stereoLinked) && !linkPolicy_.has_value()) {
        throw std::logic_error("Linked peak detector requires an explicit link policy");
    }

    DetectorFrameOutput output {};
    std::array<double, 2> magnitudes {};

    for (std::size_t channel = 0; channel < input.numChannels; ++channel) {
        const auto filtered = (filterBank_ != nullptr) ? filterBank_->process(channel, input.samples[channel]) : input.samples[channel];
        auto magnitude = std::abs(filtered);
        if (peakEmphasis_) {
            magnitude = std::min(4.0, magnitude * (1.0 + (2.0 * std::abs(filtered - envelope_[channel]))));
        }

        output.sidechainTap[channel] = filtered;
        output.rawState[channel] = magnitude;
        magnitudes[channel] = magnitude;
    }

    if (spec_.channelMode == core::ChannelMode::stereoLinked) {
        const auto linked = linkedLevel(magnitudes, input.numChannels);
        const auto linkedEnvelope = processOneChannel(0, linked);
        for (std::size_t channel = 0; channel < input.numChannels; ++channel) {
            output.linearEnvelope[channel] = linkedEnvelope;
            output.dbEnvelope[channel] = core::linearToDb(linkedEnvelope);
        }
        envelope_[1] = envelope_[0];
        holdCounters_[1] = holdCounters_[0];
        return output;
    }

    for (std::size_t channel = 0; channel < input.numChannels; ++channel) {
        const auto result = processOneChannel(channel, magnitudes[channel]);
        output.linearEnvelope[channel] = result;
        output.dbEnvelope[channel] = core::linearToDb(result);
    }
    return output;
}

double PeakDetector::attackCoefficient() const noexcept {
    return attackCoefficient_;
}

double PeakDetector::releaseCoefficient() const noexcept {
    return releaseCoefficient_;
}

double PeakDetector::linkedLevel(const std::array<double, 2>& rectified, std::size_t numChannels) const {
    if (!linkPolicy_.has_value()) {
        return rectified[0];
    }

    switch (*linkPolicy_) {
        case DetectorLinkPolicy::maximum:
            return *std::max_element(rectified.begin(), rectified.begin() + static_cast<std::ptrdiff_t>(numChannels));
        case DetectorLinkPolicy::energySum: {
            auto sum = 0.0;
            for (std::size_t channel = 0; channel < numChannels; ++channel) {
                sum += rectified[channel] * rectified[channel];
            }
            return std::sqrt(sum / static_cast<double>(numChannels));
        }
    }
    return rectified[0];
}

double PeakDetector::processOneChannel(std::size_t channelIndex, double sampleMagnitude) {
    auto& currentEnvelope = envelope_.at(channelIndex);
    auto& holdCounter = holdCounters_.at(channelIndex);
    const auto coefficient = (sampleMagnitude > currentEnvelope) ? attackCoefficient_ : releaseCoefficient_;

    if ((sampleMagnitude < currentEnvelope) && (holdCounter > 0U)) {
        --holdCounter;
        return currentEnvelope;
    }

    currentEnvelope = core::sanitizeSample((coefficient * currentEnvelope) + ((1.0 - coefficient) * sampleMagnitude));
    if (sampleMagnitude >= currentEnvelope) {
        holdCounter = holdSamples_;
    }
    return currentEnvelope;
}

void RmsDetector::prepare(const core::PrepareSpec& spec) {
    spec_ = spec;
    if (filterBank_ == nullptr) {
        filterBank_ = new SidechainFilterBank();
    }
    filterBank_->prepare(spec.sampleRate);
    setTiming(timing_);
    reset();
}

void RmsDetector::reset(double initialLevel) {
    const auto initialSquared = initialLevel * initialLevel;
    rmsSquared_.fill(initialSquared);
    postEnvelope_.fill(initialLevel);
    if (filterBank_ != nullptr) {
        filterBank_->reset();
    }
}

void RmsDetector::setTiming(DetectorTiming timing) {
    timing_ = timing;
    rmsCoefficient_ = core::timeToCoefficient(timing.rmsWindowSeconds, spec_.sampleRate);
    attackCoefficient_ = core::timeToCoefficient(timing.attackSeconds, spec_.sampleRate);
    releaseCoefficient_ = core::timeToCoefficient(timing.releaseSeconds, spec_.sampleRate);
}

void RmsDetector::setSidechainConfig(SidechainFilterConfig config) {
    if (filterBank_ == nullptr) {
        return;
    }
    filterBank_->setConfig(config);
}

void RmsDetector::setChannelMode(core::ChannelMode channelMode) {
    spec_.channelMode = channelMode;
}

void RmsDetector::setLinkPolicy(std::optional<DetectorLinkPolicy> policy) {
    linkPolicy_ = policy;
}

void RmsDetector::enablePostSmoothing(bool enabled) {
    postSmoothingEnabled_ = enabled;
}

DetectorFrameOutput RmsDetector::processFrame(const DetectorFrameInput& input) {
    if ((spec_.channelMode == core::ChannelMode::stereoLinked) && !linkPolicy_.has_value()) {
        throw std::logic_error("Linked RMS detector requires an explicit link policy");
    }

    DetectorFrameOutput output {};
    std::array<double, 2> filtered {};
    std::array<double, 2> squared {};

    for (std::size_t channel = 0; channel < input.numChannels; ++channel) {
        filtered[channel] = (filterBank_ != nullptr) ? filterBank_->process(channel, input.samples[channel]) : input.samples[channel];
        squared[channel] = filtered[channel] * filtered[channel];
        output.sidechainTap[channel] = filtered[channel];
    }

    if (spec_.channelMode == core::ChannelMode::stereoLinked) {
        const auto linkedSquared = linkedLevel(squared, input.numChannels);
        rmsSquared_[0] = (rmsCoefficient_ * rmsSquared_[0]) + ((1.0 - rmsCoefficient_) * linkedSquared);
        const auto target = std::sqrt(std::max(0.0, rmsSquared_[0]));
        const auto result = postSmoothingEnabled_ ? postSmooth(0, target) : target;

        for (std::size_t channel = 0; channel < input.numChannels; ++channel) {
            output.rawState[channel] = rmsSquared_[0];
            output.linearEnvelope[channel] = result;
            output.dbEnvelope[channel] = core::linearToDb(result);
        }
        rmsSquared_[1] = rmsSquared_[0];
        postEnvelope_[1] = postEnvelope_[0];
        return output;
    }

    for (std::size_t channel = 0; channel < input.numChannels; ++channel) {
        rmsSquared_[channel] = (rmsCoefficient_ * rmsSquared_[channel]) + ((1.0 - rmsCoefficient_) * squared[channel]);
        auto result = std::sqrt(std::max(0.0, rmsSquared_[channel]));
        if (postSmoothingEnabled_) {
            result = postSmooth(channel, result);
        }
        output.rawState[channel] = rmsSquared_[channel];
        output.linearEnvelope[channel] = result;
        output.dbEnvelope[channel] = core::linearToDb(result);
    }

    return output;
}

double RmsDetector::rmsCoefficient() const noexcept {
    return rmsCoefficient_;
}

double RmsDetector::attackCoefficient() const noexcept {
    return attackCoefficient_;
}

double RmsDetector::releaseCoefficient() const noexcept {
    return releaseCoefficient_;
}

double RmsDetector::linkedLevel(const std::array<double, 2>& squared, std::size_t numChannels) const {
    if (!linkPolicy_.has_value()) {
        return squared[0];
    }

    switch (*linkPolicy_) {
        case DetectorLinkPolicy::maximum:
            return *std::max_element(squared.begin(), squared.begin() + static_cast<std::ptrdiff_t>(numChannels));
        case DetectorLinkPolicy::energySum: {
            auto sum = 0.0;
            for (std::size_t channel = 0; channel < numChannels; ++channel) {
                sum += squared[channel];
            }
            return sum / static_cast<double>(numChannels);
        }
    }
    return squared[0];
}

double RmsDetector::postSmooth(std::size_t channelIndex, double targetLevel) {
    auto& envelope = postEnvelope_.at(channelIndex);
    const auto coefficient = (targetLevel > envelope) ? attackCoefficient_ : releaseCoefficient_;
    envelope = core::sanitizeSample((coefficient * envelope) + ((1.0 - coefficient) * targetLevel));
    return envelope;
}

}  // namespace phantom::dsp
