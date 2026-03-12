#include "tests/TestRunner.h"

#include "core/processing/Types.h"
#include "dsp/detectors/Detectors.h"

#include <cmath>
#include <numbers>

namespace phantom::testing {

namespace {

double drivePeak(phantom::dsp::PeakDetector& detector, double value, std::size_t samples) {
    phantom::dsp::DetectorFrameOutput output {};
    for (std::size_t index = 0; index < samples; ++index) {
        output = detector.processFrame({.samples = {value, value}, .numChannels = 1});
    }
    return output.linearEnvelope[0];
}

}  // namespace

void runDetectorTests() {
    using namespace phantom;

    const core::PrepareSpec spec {
        .sampleRate = 48000.0,
        .maxBlockSize = 128,
        .numChannels = 2,
        .channelMode = core::ChannelMode::stereoLinked
    };

    dsp::PeakDetector peakDetector;
    peakDetector.prepare(spec);
    peakDetector.setLinkPolicy(dsp::DetectorLinkPolicy::maximum);
    peakDetector.setTiming({.attackSeconds = 0.001, .releaseSeconds = 0.010, .rmsWindowSeconds = 0.010});

    const auto levelAfterAttack = drivePeak(peakDetector, 1.0, 48);
    PHANTOM_EXPECT_TRUE(levelAfterAttack > 0.55);

    peakDetector.setSidechainConfig({
        .mode = dsp::SidechainFilterMode::highPass,
        .frequencyHz = 400.0,
        .q = 0.70710678118
    });
    peakDetector.reset();

    auto bypassDetector = dsp::PeakDetector {};
    bypassDetector.prepare(spec);
    bypassDetector.setLinkPolicy(dsp::DetectorLinkPolicy::maximum);
    bypassDetector.setTiming({.attackSeconds = 0.001, .releaseSeconds = 0.010, .rmsWindowSeconds = 0.010});

    double highPassedEnvelope = 0.0;
    double bypassEnvelope = 0.0;
    for (std::size_t index = 0; index < 512; ++index) {
        const auto sample = std::sin((2.0 * std::numbers::pi * 40.0 * static_cast<double>(index)) / spec.sampleRate);
        highPassedEnvelope = peakDetector.processFrame({.samples = {sample, sample}, .numChannels = 1}).linearEnvelope[0];
        bypassEnvelope = bypassDetector.processFrame({.samples = {sample, sample}, .numChannels = 1}).linearEnvelope[0];
    }
    PHANTOM_EXPECT_TRUE(highPassedEnvelope < bypassEnvelope);

    dsp::RmsDetector rmsDetector;
    rmsDetector.prepare({
        .sampleRate = 96000.0,
        .maxBlockSize = 64,
        .numChannels = 2,
        .channelMode = core::ChannelMode::stereoLinked
    });
    rmsDetector.setLinkPolicy(dsp::DetectorLinkPolicy::energySum);
    rmsDetector.enablePostSmoothing(true);

    auto linkedOutput = rmsDetector.processFrame({.samples = {0.5, 0.25}, .numChannels = 2});
    PHANTOM_EXPECT_TRUE(linkedOutput.linearEnvelope[0] > linkedOutput.linearEnvelope[1] - 1.0e-9);
    PHANTOM_EXPECT_TRUE(rmsDetector.attackCoefficient() < rmsDetector.releaseCoefficient());
}

}  // namespace phantom::testing
