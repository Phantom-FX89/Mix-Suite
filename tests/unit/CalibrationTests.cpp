#include "tests/TestRunner.h"

#include "core/calibration/Calibration.h"

namespace phantom::testing {

void runCalibrationTests() {
    using namespace phantom::core;

    CalibrationSystem calibration;
    calibration.setReferenceMode(CalibrationReferenceMode::vuMinus20Dbfs);
    calibration.registerPluginProfile({
        .pluginId = "shared-test",
        .nominalOffsetFromReferenceDb = 0.0,
        .nonlinearOnsetOffsetDb = 6.0,
        .softSatOffsetDb = 10.0,
        .clipOffsetDb = 18.0
    });

    const auto resolved = calibration.resolveProfile("shared-test");
    PHANTOM_EXPECT_TRUE(resolved.has_value());
    PHANTOM_EXPECT_NEAR(resolved->nominalLevelDbfs, -20.0, 1.0e-9);
    PHANTOM_EXPECT_NEAR(resolved->nonlinearOnsetDbfs, -14.0, 1.0e-9);
    PHANTOM_EXPECT_NEAR(resolved->softSatOnsetDbfs, -10.0, 1.0e-9);
    PHANTOM_EXPECT_NEAR(resolved->clipThresholdDbfs, -2.0, 1.0e-9);

    PHANTOM_EXPECT_NEAR(calibration.dbfsFromVu(3.0), -17.0, 1.0e-9);
    PHANTOM_EXPECT_NEAR(calibration.normalizedDriveAboveNominal(-15.0, *resolved), 0.5, 1.0e-9);
    PHANTOM_EXPECT_NEAR(calibration.normalizedSaturationProximity(-6.0, *resolved), 0.5, 1.0e-9);
    PHANTOM_EXPECT_TRUE(calibration.classifyLevel(-1.0, *resolved) == DriveRegion::clipping);
}

}  // namespace phantom::testing
