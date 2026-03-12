#include "core/calibration/Calibration.h"

#include <algorithm>
#include <stdexcept>

namespace phantom::core {

namespace {

constexpr double kNearNominalWindowDb = 3.0;

}  // namespace

void CalibrationSystem::setReferenceMode(CalibrationReferenceMode mode) noexcept {
    mode_ = mode;
}

CalibrationReferenceMode CalibrationSystem::referenceMode() const noexcept {
    return mode_;
}

double CalibrationSystem::referenceOffsetDbfs() const noexcept {
    return phantom::core::referenceOffsetDbfs(mode_);
}

void CalibrationSystem::registerPluginProfile(PluginCalibrationProfile profile) {
    if (profile.pluginId.empty()) {
        throw std::invalid_argument("Calibration profile requires a plugin id");
    }
    profiles_.insert_or_assign(profile.pluginId, std::move(profile));
}

bool CalibrationSystem::hasProfile(std::string_view pluginId) const {
    return profiles_.contains(std::string(pluginId));
}

std::optional<ResolvedCalibrationProfile> CalibrationSystem::resolveProfile(std::string_view pluginId) const {
    const auto iterator = profiles_.find(std::string(pluginId));
    if (iterator == profiles_.end()) {
        return std::nullopt;
    }
    return resolveProfile(iterator->second);
}

ResolvedCalibrationProfile CalibrationSystem::resolveProfile(const PluginCalibrationProfile& profile) const {
    const auto referenceDbfs = referenceOffsetDbfs();
    return {
        .pluginId = profile.pluginId,
        .nominalLevelDbfs = referenceDbfs + profile.nominalOffsetFromReferenceDb,
        .nonlinearOnsetDbfs = referenceDbfs + profile.nonlinearOnsetOffsetDb,
        .softSatOnsetDbfs = referenceDbfs + profile.softSatOffsetDb,
        .clipThresholdDbfs = referenceDbfs + profile.clipOffsetDb,
        .cleanHeadroomMarginDb = profile.cleanHeadroomMarginDb
    };
}

double CalibrationSystem::dbfsFromVu(double vuValue) const noexcept {
    return referenceOffsetDbfs() + vuValue;
}

double CalibrationSystem::normalizedDriveAboveNominal(double signalDbfs, const ResolvedCalibrationProfile& profile) const noexcept {
    const auto spanDb = std::max(profile.softSatOnsetDbfs - profile.nominalLevelDbfs, 1.0e-6);
    return std::max(0.0, (signalDbfs - profile.nominalLevelDbfs) / spanDb);
}

double CalibrationSystem::normalizedSaturationProximity(double signalDbfs, const ResolvedCalibrationProfile& profile) const noexcept {
    const auto spanDb = std::max(profile.clipThresholdDbfs - profile.softSatOnsetDbfs, 1.0e-6);
    return std::clamp((signalDbfs - profile.softSatOnsetDbfs) / spanDb, 0.0, 1.0);
}

DriveRegion CalibrationSystem::classifyLevel(double signalDbfs, const ResolvedCalibrationProfile& profile) const noexcept {
    if (signalDbfs >= profile.clipThresholdDbfs) {
        return DriveRegion::clipping;
    }
    if (signalDbfs >= profile.softSatOnsetDbfs) {
        return DriveRegion::driven;
    }
    if (signalDbfs >= (profile.nominalLevelDbfs - kNearNominalWindowDb)) {
        return DriveRegion::nearNominal;
    }
    return DriveRegion::belowNominal;
}

const char* toString(CalibrationReferenceMode mode) noexcept {
    switch (mode) {
        case CalibrationReferenceMode::vuMinus18Dbfs:
            return "0 VU = -18 dBFS";
        case CalibrationReferenceMode::vuMinus20Dbfs:
            return "0 VU = -20 dBFS";
        case CalibrationReferenceMode::vuMinus14Dbfs:
            return "0 VU = -14 dBFS";
    }
    return "Unknown";
}

double referenceOffsetDbfs(CalibrationReferenceMode mode) noexcept {
    switch (mode) {
        case CalibrationReferenceMode::vuMinus18Dbfs:
            return -18.0;
        case CalibrationReferenceMode::vuMinus20Dbfs:
            return -20.0;
        case CalibrationReferenceMode::vuMinus14Dbfs:
            return -14.0;
    }
    return -18.0;
}

}  // namespace phantom::core
