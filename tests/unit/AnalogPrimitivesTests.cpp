#include "tests/TestRunner.h"

#include "dsp/analog/AnalogPrimitives.h"

namespace phantom::testing {

void runAnalogPrimitivesTests() {
    using namespace phantom::dsp;

    ComponentVariance variance;
    variance.setSeed(1234u);
    const auto resistorA = variance.multiplierFor("R1", 0.05);
    const auto resistorB = variance.multiplierFor("R1", 0.05);
    PHANTOM_EXPECT_NEAR(resistorA, resistorB, 1.0e-12);

    WhiteNoiseGenerator noise;
    noise.setSeed(42u);
    noise.setLevelDb(-90.0);
    const auto sampleA = noise.nextSample();
    const auto sampleB = noise.nextSample();
    PHANTOM_EXPECT_TRUE(sampleA != sampleB);

    StereoCrosstalk crosstalk;
    crosstalk.setAmountDb(-30.0);
    const auto frame = crosstalk.process(1.0, 0.0);
    PHANTOM_EXPECT_TRUE(frame[1] > 0.0);

    PowerSupplyMemory supply;
    supply.prepare(48000.0);
    const auto first = supply.process(1.0);
    const auto second = supply.process(0.0);
    PHANTOM_EXPECT_TRUE(first > second);
}

}  // namespace phantom::testing
