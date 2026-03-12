#include <iostream>
#include <vector>

namespace phantom::testing {

void runAnalogPrimitivesTests();
void runCalibrationTests();
void runDetectorTests();
void runGainComputerTests();
void runNonlinearTests();
void runOversamplingTests();

}  // namespace phantom::testing

int main() {
    const std::vector<void (*)()> tests = {
        &phantom::testing::runCalibrationTests,
        &phantom::testing::runDetectorTests,
        &phantom::testing::runGainComputerTests,
        &phantom::testing::runOversamplingTests,
        &phantom::testing::runNonlinearTests,
        &phantom::testing::runAnalogPrimitivesTests
    };

    try {
        for (const auto& test : tests) {
            test();
        }
        std::cout << "phantom_dsp_tests: all tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "phantom_dsp_tests: " << error.what() << "\n";
        return 1;
    }
}
