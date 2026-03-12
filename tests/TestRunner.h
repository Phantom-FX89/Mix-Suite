#pragma once

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>

namespace phantom::testing {

class TestFailure : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

inline void fail(const char* expression, const char* file, int line, const std::string& message = {}) {
    std::ostringstream stream;
    stream << file << ":" << line << " -> " << expression;
    if (!message.empty()) {
        stream << " (" << message << ")";
    }
    throw TestFailure(stream.str());
}

inline void expectTrue(bool condition, const char* expression, const char* file, int line) {
    if (!condition) {
        fail(expression, file, line);
    }
}

inline void expectNear(double actual, double expected, double tolerance, const char* expression, const char* file, int line) {
    if (std::abs(actual - expected) > tolerance) {
        std::ostringstream stream;
        stream << "expected " << expected << " +/- " << tolerance << ", got " << actual;
        fail(expression, file, line, stream.str());
    }
}

}  // namespace phantom::testing

#define PHANTOM_EXPECT_TRUE(expr) ::phantom::testing::expectTrue((expr), #expr, __FILE__, __LINE__)
#define PHANTOM_EXPECT_NEAR(actual, expected, tolerance) \
    ::phantom::testing::expectNear((actual), (expected), (tolerance), #actual, __FILE__, __LINE__)
