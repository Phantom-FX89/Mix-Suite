#include "dsp/nonlinear/Nonlinear.h"

#include <algorithm>
#include <cmath>

namespace phantom::dsp {

namespace {

[[nodiscard]] double safeDriveScale(const core::ResolvedCalibrationProfile& calibration, const DriveContext& driveContext) {
    const auto span = std::max(1.0, calibration.softSatOnsetDbfs - calibration.nominalLevelDbfs);
    const auto driveFromLevel = std::max(0.0, (driveContext.inputLevelDbfs - calibration.nominalLevelDbfs) / span);
    return std::max(driveContext.normalizedDrive, driveFromLevel);
}

[[nodiscard]] double activity(double inputSample, double outputSample) {
    return std::abs(outputSample - inputSample);
}

void fillTelemetry(StageTelemetry* telemetry, double inputSample, double outputSample, double saturationProximity) {
    if (telemetry == nullptr) {
        return;
    }
    telemetry->harmonicActivityEstimate = activity(inputSample, outputSample);
    telemetry->saturationProximity = std::clamp(saturationProximity, 0.0, 1.0);
    telemetry->debugPreShape = inputSample;
    telemetry->debugPostShape = outputSample;
}

}  // namespace

void NonlinearStage::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
}

void NonlinearStage::reset() {}

void TransformerStage::setDrive(double drive) noexcept {
    drive_ = drive;
}

void TransformerStage::setAsymmetry(double asymmetry) noexcept {
    asymmetry_ = asymmetry;
}

void TransformerStage::setMemoryAmount(double amount) noexcept {
    memoryAmount_ = amount;
}

core::OversamplingMode TransformerStage::preferredOversamplingMode() const noexcept {
    return core::OversamplingMode::x2;
}

NonlinearFamily TransformerStage::family() const noexcept {
    return NonlinearFamily::transformer;
}

double TransformerStage::processSample(double inputSample,
                                       const core::ResolvedCalibrationProfile& calibration,
                                       const DriveContext& driveContext,
                                       StageTelemetry* telemetry) {
    const auto memoryCoefficient = core::timeToCoefficient(0.080, sampleRate_);
    memoryState_ = (memoryCoefficient * memoryState_) + ((1.0 - memoryCoefficient) * inputSample);

    const auto effectiveDrive = drive_ * (1.0 + (0.7 * safeDriveScale(calibration, driveContext)));
    const auto shapedInput = inputSample + (memoryAmount_ * memoryState_) + (asymmetry_ * 0.1);
    const auto outputSample = std::tanh(effectiveDrive * shapedInput) / std::tanh(std::max(1.0, effectiveDrive));

    fillTelemetry(telemetry, inputSample, outputSample, driveContext.normalizedDrive);
    return core::sanitizeSample(outputSample);
}

void TransformerStage::reset() {
    memoryState_ = 0.0;
}

void TubeStage::setAsymmetry(double asymmetry) noexcept {
    asymmetry_ = asymmetry;
}

core::OversamplingMode TubeStage::preferredOversamplingMode() const noexcept {
    return core::OversamplingMode::x2;
}

NonlinearFamily TubeStage::family() const noexcept {
    return NonlinearFamily::tube;
}

double TubeStage::processSample(double inputSample,
                                const core::ResolvedCalibrationProfile& calibration,
                                const DriveContext& driveContext,
                                StageTelemetry* telemetry) {
    const auto driveScale = 1.0 + (0.9 * safeDriveScale(calibration, driveContext));
    const auto biased = inputSample * driveScale + (asymmetry_ * 0.1);
    const auto outputSample = (1.0 * biased) + (0.10 * biased * biased) - (0.12 * std::pow(biased, 3.0)) + (0.02 * std::pow(biased, 5.0));

    fillTelemetry(telemetry, inputSample, outputSample, driveContext.normalizedDrive);
    return core::sanitizeSample(std::tanh(outputSample));
}

void VcaStage::setColourAmount(double amount) noexcept {
    colourAmount_ = amount;
}

core::OversamplingMode VcaStage::preferredOversamplingMode() const noexcept {
    return core::OversamplingMode::x1;
}

NonlinearFamily VcaStage::family() const noexcept {
    return NonlinearFamily::vca;
}

double VcaStage::processSample(double inputSample,
                               const core::ResolvedCalibrationProfile&,
                               const DriveContext& driveContext,
                               StageTelemetry* telemetry) {
    const auto cubic = colourAmount_ * driveContext.gainReductionNormalized * std::pow(inputSample, 3.0);
    const auto outputSample = inputSample + cubic;
    fillTelemetry(telemetry, inputSample, outputSample, driveContext.gainReductionNormalized);
    return core::sanitizeSample(outputSample);
}

void FetStage::setBaseDrive(double drive) noexcept {
    baseDrive_ = drive;
}

void FetStage::setInputDriveCoefficient(double coefficient) noexcept {
    inputDriveCoefficient_ = coefficient;
}

void FetStage::setGainReductionDriveCoefficient(double coefficient) noexcept {
    gainReductionDriveCoefficient_ = coefficient;
}

void FetStage::setAsymmetry(double asymmetry) noexcept {
    asymmetry_ = asymmetry;
}

core::OversamplingMode FetStage::preferredOversamplingMode() const noexcept {
    return core::OversamplingMode::x4;
}

NonlinearFamily FetStage::family() const noexcept {
    return NonlinearFamily::fet;
}

double FetStage::processSample(double inputSample,
                               const core::ResolvedCalibrationProfile& calibration,
                               const DriveContext& driveContext,
                               StageTelemetry* telemetry) {
    const auto drive = baseDrive_ + (inputDriveCoefficient_ * safeDriveScale(calibration, driveContext)) +
                       (gainReductionDriveCoefficient_ * driveContext.gainReductionNormalized);
    const auto biased = inputSample + (asymmetry_ * inputSample);
    const auto outputSample = biased / (1.0 + (drive * std::abs(biased)));

    fillTelemetry(telemetry, inputSample, outputSample, driveContext.gainReductionNormalized);
    return core::sanitizeSample(outputSample);
}

void DiodeZenerStage::setSharpness(double sharpness) noexcept {
    sharpness_ = sharpness;
}

core::OversamplingMode DiodeZenerStage::preferredOversamplingMode() const noexcept {
    return core::OversamplingMode::x4;
}

NonlinearFamily DiodeZenerStage::family() const noexcept {
    return NonlinearFamily::diodeZener;
}

double DiodeZenerStage::processSample(double inputSample,
                                      const core::ResolvedCalibrationProfile& calibration,
                                      const DriveContext& driveContext,
                                      StageTelemetry* telemetry) {
    const auto drive = sharpness_ * (1.0 + safeDriveScale(calibration, driveContext));
    const auto magnitude = 1.0 - std::exp(-drive * std::abs(inputSample));
    const auto outputSample = std::copysign(magnitude, inputSample);

    fillTelemetry(telemetry, inputSample, outputSample, driveContext.normalizedDrive);
    return core::sanitizeSample(outputSample);
}

void VariMuStage::setDensity(double density) noexcept {
    density_ = density;
}

core::OversamplingMode VariMuStage::preferredOversamplingMode() const noexcept {
    return core::OversamplingMode::x4;
}

NonlinearFamily VariMuStage::family() const noexcept {
    return NonlinearFamily::variMu;
}

double VariMuStage::processSample(double inputSample,
                                  const core::ResolvedCalibrationProfile& calibration,
                                  const DriveContext& driveContext,
                                  StageTelemetry* telemetry) {
    const auto drive = density_ * (1.0 + (0.6 * safeDriveScale(calibration, driveContext)) + (0.3 * driveContext.gainReductionNormalized));
    const auto outputSample = std::tanh(inputSample * (1.0 + drive));

    fillTelemetry(telemetry, inputSample, outputSample, driveContext.normalizedDrive);
    return core::sanitizeSample(outputSample);
}

}  // namespace phantom::dsp
