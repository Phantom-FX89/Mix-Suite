cmake_minimum_required(VERSION 3.22)

project(PhantomMixSuiteDSP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
    phantom_dsp
    STATIC
    core/calibration/Calibration.cpp
    core/metering/Metering.cpp
    core/oversampling/Oversampling.cpp
    dsp/analog/AnalogPrimitives.cpp
    dsp/detectors/Detectors.cpp
    dsp/filters/Biquad.cpp
    dsp/gain/GainComputer.cpp
    dsp/nonlinear/Nonlinear.cpp
)

target_include_directories(phantom_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(phantom_dsp PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(phantom_dsp PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(
    phantom_dsp_tests
    tests/TestMain.cpp
    tests/unit/AnalogPrimitivesTests.cpp
    tests/unit/CalibrationTests.cpp
    tests/unit/DetectorTests.cpp
    tests/unit/GainComputerTests.cpp
    tests/unit/NonlinearTests.cpp
    tests/unit/OversamplingTests.cpp
)

target_link_libraries(phantom_dsp_tests PRIVATE phantom_dsp)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(phantom_dsp_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()

add_test(NAME phantom_dsp_tests COMMAND phantom_dsp_tests)
