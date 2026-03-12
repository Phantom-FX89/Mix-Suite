#pragma once

#include "core/processing/Types.h"
#include "dsp/filters/Biquad.h"

#include <array>
#include <cstddef>
#include <optional>

namespace phantom::dsp {

enum class DetectorLinkPolicy {
    maximum,
    energySum
};

enum class DetectorSourcePosition {
    preGain,
    postGain
};

enum class SidechainFilterMode {
    bypass,
    highPass,
    tilt,
    bell
};

struct DetectorTiming {
    double attackSeconds {0.001};
    double releaseSeconds {0.05};
    double rmsWindowSeconds {0.01};
};

struct SidechainFilterConfig {
    SidechainFilterMode mode {SidechainFilterMode::bypass};
    double frequencyHz {120.0};
    double q {0.70710678118};
    double gainDb {0.0};
    double tiltDb {0.0};
};

struct DetectorFrameInput {
    std::array<double, 2> samples {};
    std::size_t numChannels {1};
};

struct DetectorFrameOutput {
    std::array<double, 2> linearEnvelope {};
    std::array<double, 2> dbEnvelope {};
    std::array<double, 2> sidechainTap {};
    std::array<double, 2> rawState {};
};

struct ProgramDependentDetectorState {
    double exposure {0.0};
    double releaseStageA {0.0};
    double releaseStageB {0.0};
};

class ProgramDependentDetectorScaffold {
public:
    void prepare(double sampleRate);
    void reset();
    [[nodiscard]] ProgramDependentDetectorState processSample(double levelLinear);
    [[nodiscard]] const ProgramDependentDetectorState& state() const noexcept;

private:
    double sampleRate_ {48000.0};
    ProgramDependentDetectorState state_ {};
};

class PeakDetector {
public:
    PeakDetector() = default;
    ~PeakDetector();
    PeakDetector(const PeakDetector&) = delete;
    PeakDetector& operator=(const PeakDetector&) = delete;
    PeakDetector(PeakDetector&&) = delete;
    PeakDetector& operator=(PeakDetector&&) = delete;

    void prepare(const core::PrepareSpec& spec);
    void reset(double initialLevel = 0.0);
    void setTiming(DetectorTiming timing);
    void setSidechainConfig(SidechainFilterConfig config);
    void setChannelMode(core::ChannelMode channelMode);
    void setLinkPolicy(std::optional<DetectorLinkPolicy> policy);
    void setPeakEmphasis(bool enabled);
    void setHoldSamples(std::size_t holdSamples);

    [[nodiscard]] DetectorFrameOutput processFrame(const DetectorFrameInput& input);
    [[nodiscard]] double attackCoefficient() const noexcept;
    [[nodiscard]] double releaseCoefficient() const noexcept;

private:
    class SidechainFilterBank;

    [[nodiscard]] double linkedLevel(const std::array<double, 2>& rectified, std::size_t numChannels) const;
    [[nodiscard]] double processOneChannel(std::size_t channelIndex, double sampleMagnitude);

    core::PrepareSpec spec_ {};
    DetectorTiming timing_ {};
    std::optional<DetectorLinkPolicy> linkPolicy_;
    bool peakEmphasis_ {false};
    std::size_t holdSamples_ {0};
    double attackCoefficient_ {0.0};
    double releaseCoefficient_ {0.0};
    std::array<double, 2> envelope_ {};
    std::array<std::size_t, 2> holdCounters_ {};
    SidechainFilterBank* filterBank_ {nullptr};
};

class RmsDetector {
public:
    RmsDetector() = default;
    ~RmsDetector();
    RmsDetector(const RmsDetector&) = delete;
    RmsDetector& operator=(const RmsDetector&) = delete;
    RmsDetector(RmsDetector&&) = delete;
    RmsDetector& operator=(RmsDetector&&) = delete;

    void prepare(const core::PrepareSpec& spec);
    void reset(double initialLevel = 0.0);
    void setTiming(DetectorTiming timing);
    void setSidechainConfig(SidechainFilterConfig config);
    void setChannelMode(core::ChannelMode channelMode);
    void setLinkPolicy(std::optional<DetectorLinkPolicy> policy);
    void enablePostSmoothing(bool enabled);

    [[nodiscard]] DetectorFrameOutput processFrame(const DetectorFrameInput& input);
    [[nodiscard]] double rmsCoefficient() const noexcept;
    [[nodiscard]] double attackCoefficient() const noexcept;
    [[nodiscard]] double releaseCoefficient() const noexcept;

private:
    class SidechainFilterBank;

    [[nodiscard]] double linkedLevel(const std::array<double, 2>& squared, std::size_t numChannels) const;
    [[nodiscard]] double postSmooth(std::size_t channelIndex, double targetLevel);

    core::PrepareSpec spec_ {};
    DetectorTiming timing_ {};
    std::optional<DetectorLinkPolicy> linkPolicy_;
    bool postSmoothingEnabled_ {false};
    double rmsCoefficient_ {0.0};
    double attackCoefficient_ {0.0};
    double releaseCoefficient_ {0.0};
    std::array<double, 2> rmsSquared_ {};
    std::array<double, 2> postEnvelope_ {};
    SidechainFilterBank* filterBank_ {nullptr};
};

}  // namespace phantom::dsp
