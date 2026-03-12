#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace phantom::core {

enum class CalibrationReferenceMode {
    vuMinus18Dbfs,
    vuMinus20Dbfs,
    vuMinus14Dbfs
};

enum class DriveRegion {
    belowNominal,
    nearNominal,
    driven,
    clipping
};

struct PluginCalibrationProfile {
    std::string pluginId;
    double nominalOffsetFromReferenceDb {0.0};
    double nonlinearOnsetOffsetDb {6.0};
    double softSatOffsetDb {10.0};
    double clipOffsetDb {18.0};
    std::optional<double> cleanHeadroomMarginDb;
};

struct ResolvedCalibrationProfile {
    std::string pluginId;
    double nominalLevelDbfs {0.0};
    double nonlinearOnsetDbfs {0.0};
    double softSatOnsetDbfs {0.0};
    double clipThresholdDbfs {0.0};
    std::optional<double> cleanHeadroomMarginDb;
};

class CalibrationSystem {
public:
    void setReferenceMode(CalibrationReferenceMode mode) noexcept;
    [[nodiscard]] CalibrationReferenceMode referenceMode() const noexcept;
    [[nodiscard]] double referenceOffsetDbfs() const noexcept;

    void registerPluginProfile(PluginCalibrationProfile profile);
    [[nodiscard]] bool hasProfile(std::string_view pluginId) const;
    [[nodiscard]] std::optional<ResolvedCalibrationProfile> resolveProfile(std::string_view pluginId) const;
    [[nodiscard]] ResolvedCalibrationProfile resolveProfile(const PluginCalibrationProfile& profile) const;

    [[nodiscard]] double dbfsFromVu(double vuValue) const noexcept;
    [[nodiscard]] double normalizedDriveAboveNominal(double signalDbfs, const ResolvedCalibrationProfile& profile) const noexcept;
    [[nodiscard]] double normalizedSaturationProximity(double signalDbfs, const ResolvedCalibrationProfile& profile) const noexcept;
    [[nodiscard]] DriveRegion classifyLevel(double signalDbfs, const ResolvedCalibrationProfile& profile) const noexcept;

private:
    CalibrationReferenceMode mode_ {CalibrationReferenceMode::vuMinus18Dbfs};
    std::unordered_map<std::string, PluginCalibrationProfile> profiles_;
};

[[nodiscard]] const char* toString(CalibrationReferenceMode mode) noexcept;
[[nodiscard]] double referenceOffsetDbfs(CalibrationReferenceMode mode) noexcept;

}  // namespace phantom::core
