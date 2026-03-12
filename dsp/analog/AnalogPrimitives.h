#pragma once

#include "core/processing/Types.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace phantom::dsp {

class ComponentVariance {
public:
    void setSeed(std::uint32_t seed) noexcept;
    [[nodiscard]] double multiplierFor(std::string_view componentId, double spreadFraction) const noexcept;

private:
    std::uint32_t seed_ {0xC0FFEEu};
};

class WhiteNoiseGenerator {
public:
    void setSeed(std::uint32_t seed) noexcept;
    void setLevelDb(double levelDb) noexcept;
    [[nodiscard]] double nextSample() noexcept;

private:
    std::uint32_t state_ {0xBAD5EEDu};
    double levelLinear_ {0.0};
};

class StereoCrosstalk {
public:
    void setAmountDb(double amountDb) noexcept;
    [[nodiscard]] std::array<double, 2> process(double left, double right) const noexcept;

private:
    double coefficient_ {0.0};
};

class PowerSupplyMemory {
public:
    void prepare(double sampleRate);
    void reset();
    [[nodiscard]] double process(double normalizedDrive);

private:
    double sampleRate_ {48000.0};
    double state_ {0.0};
    double attackCoefficient_ {0.0};
    double releaseCoefficient_ {0.0};
};

}  // namespace phantom::dsp
