#pragma once

#include "core/calibration/Calibration.h"
#include "core/processing/Types.h"

namespace phantom::dsp {

enum class NonlinearFamily {
    transformer,
    tube,
    vca,
    fet,
    diodeZener,
    variMu
};

struct DriveContext {
    double inputLevelDbfs {-60.0};
    double normalizedDrive {0.0};
    double gainReductionDb {0.0};
    double gainReductionNormalized {0.0};
    double effectiveSampleRate {48000.0};
    std::size_t oversamplingFactor {1};
};

struct StageTelemetry {
    double harmonicActivityEstimate {0.0};
    double saturationProximity {0.0};
    double debugPreShape {0.0};
    double debugPostShape {0.0};
};

class NonlinearStage {
public:
    virtual ~NonlinearStage() = default;

    virtual void prepare(double sampleRate);
    virtual void reset();
    [[nodiscard]] virtual core::OversamplingMode preferredOversamplingMode() const noexcept = 0;
    [[nodiscard]] virtual NonlinearFamily family() const noexcept = 0;
    [[nodiscard]] virtual double processSample(double inputSample,
                                               const core::ResolvedCalibrationProfile& calibration,
                                               const DriveContext& driveContext,
                                               StageTelemetry* telemetry = nullptr) = 0;

protected:
    double sampleRate_ {48000.0};
};

class TransformerStage final : public NonlinearStage {
public:
    void setDrive(double drive) noexcept;
    void setAsymmetry(double asymmetry) noexcept;
    void setMemoryAmount(double amount) noexcept;

    [[nodiscard]] core::OversamplingMode preferredOversamplingMode() const noexcept override;
    [[nodiscard]] NonlinearFamily family() const noexcept override;
    [[nodiscard]] double processSample(double inputSample,
                                       const core::ResolvedCalibrationProfile& calibration,
                                       const DriveContext& driveContext,
                                       StageTelemetry* telemetry = nullptr) override;
    void reset() override;

private:
    double drive_ {1.0};
    double asymmetry_ {0.05};
    double memoryAmount_ {0.15};
    double memoryState_ {0.0};
};

class TubeStage final : public NonlinearStage {
public:
    void setAsymmetry(double asymmetry) noexcept;

    [[nodiscard]] core::OversamplingMode preferredOversamplingMode() const noexcept override;
    [[nodiscard]] NonlinearFamily family() const noexcept override;
    [[nodiscard]] double processSample(double inputSample,
                                       const core::ResolvedCalibrationProfile& calibration,
                                       const DriveContext& driveContext,
                                       StageTelemetry* telemetry = nullptr) override;

private:
    double asymmetry_ {0.12};
};

class VcaStage final : public NonlinearStage {
public:
    void setColourAmount(double amount) noexcept;

    [[nodiscard]] core::OversamplingMode preferredOversamplingMode() const noexcept override;
    [[nodiscard]] NonlinearFamily family() const noexcept override;
    [[nodiscard]] double processSample(double inputSample,
                                       const core::ResolvedCalibrationProfile& calibration,
                                       const DriveContext& driveContext,
                                       StageTelemetry* telemetry = nullptr) override;

private:
    double colourAmount_ {0.015};
};

class FetStage final : public NonlinearStage {
public:
    void setBaseDrive(double drive) noexcept;
    void setInputDriveCoefficient(double coefficient) noexcept;
    void setGainReductionDriveCoefficient(double coefficient) noexcept;
    void setAsymmetry(double asymmetry) noexcept;

    [[nodiscard]] core::OversamplingMode preferredOversamplingMode() const noexcept override;
    [[nodiscard]] NonlinearFamily family() const noexcept override;
    [[nodiscard]] double processSample(double inputSample,
                                       const core::ResolvedCalibrationProfile& calibration,
                                       const DriveContext& driveContext,
                                       StageTelemetry* telemetry = nullptr) override;

private:
    double baseDrive_ {0.35};
    double inputDriveCoefficient_ {0.65};
    double gainReductionDriveCoefficient_ {1.1};
    double asymmetry_ {0.08};
};

class DiodeZenerStage final : public NonlinearStage {
public:
    void setSharpness(double sharpness) noexcept;

    [[nodiscard]] core::OversamplingMode preferredOversamplingMode() const noexcept override;
    [[nodiscard]] NonlinearFamily family() const noexcept override;
    [[nodiscard]] double processSample(double inputSample,
                                       const core::ResolvedCalibrationProfile& calibration,
                                       const DriveContext& driveContext,
                                       StageTelemetry* telemetry = nullptr) override;

private:
    double sharpness_ {2.2};
};

class VariMuStage final : public NonlinearStage {
public:
    void setDensity(double density) noexcept;

    [[nodiscard]] core::OversamplingMode preferredOversamplingMode() const noexcept override;
    [[nodiscard]] NonlinearFamily family() const noexcept override;
    [[nodiscard]] double processSample(double inputSample,
                                       const core::ResolvedCalibrationProfile& calibration,
                                       const DriveContext& driveContext,
                                       StageTelemetry* telemetry = nullptr) override;

private:
    double density_ {0.45};
};

}  // namespace phantom::dsp
