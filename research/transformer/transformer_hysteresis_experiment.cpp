#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <string_view>
#include <vector>

namespace {

constexpr double kPi = std::numbers::pi_v<double>;

[[nodiscard]] double clamp(double value, double minimum, double maximum) {
    return std::clamp(value, minimum, maximum);
}

[[nodiscard]] double linearToDb(double value) {
    return 20.0 * std::log10(std::max(value, 1.0e-12));
}

struct CandidateConfig {
    double sampleRate {48000.0};
    double drive {2.4};
    double asymmetry {0.04};
    double contourAmount {0.18};
    double memoryAmount {0.20};
    double memoryTimeSeconds {0.080};
};

class ContourFilter {
public:
    void prepare(double sampleRate) {
        const auto timeSeconds = 0.002;
        coefficient_ = std::exp(-1.0 / (timeSeconds * sampleRate));
        lowState_ = 0.0;
    }

    void reset() {
        lowState_ = 0.0;
    }

    [[nodiscard]] double process(double input, double amount) {
        lowState_ = (coefficient_ * lowState_) + ((1.0 - coefficient_) * input);
        return input + (amount * (lowState_ - input));
    }

private:
    double coefficient_ {0.0};
    double lowState_ {0.0};
};

class StaticTransformerCandidate {
public:
    explicit StaticTransformerCandidate(CandidateConfig config)
        : config_(config) {}

    void prepare() {
        contour_.prepare(config_.sampleRate);
    }

    void reset() {
        contour_.reset();
    }

    [[nodiscard]] double process(double input) {
        const auto contoured = contour_.process(input, config_.contourAmount);
        const auto biased = contoured + (config_.asymmetry * 0.06);
        const auto normalized = std::tanh(config_.drive * biased);
        return normalized / std::tanh(std::max(1.0, config_.drive));
    }

    [[nodiscard]] double memoryState() const {
        return 0.0;
    }

private:
    CandidateConfig config_;
    ContourFilter contour_;
};

class MemoryTransformerCandidate {
public:
    explicit MemoryTransformerCandidate(CandidateConfig config)
        : config_(config) {}

    void prepare() {
        contour_.prepare(config_.sampleRate);
        memoryCoefficient_ = std::exp(-1.0 / (config_.memoryTimeSeconds * config_.sampleRate));
        normalization_ = std::tanh(std::max(1.0, config_.drive * (1.0 + config_.memoryAmount)));
    }

    void reset() {
        contour_.reset();
        memoryState_ = 0.0;
        peakMemoryState_ = 0.0;
    }

    [[nodiscard]] double process(double input) {
        const auto contoured = contour_.process(input, config_.contourAmount);
        const auto memoryDriver = clamp(std::abs(contoured) * 1.2, 0.0, 1.0);
        memoryState_ = clamp((memoryCoefficient_ * memoryState_) + ((1.0 - memoryCoefficient_) * memoryDriver), 0.0, 1.0);
        peakMemoryState_ = std::max(peakMemoryState_, std::abs(memoryState_));

        const auto effectiveDrive = config_.drive * (1.0 + (config_.memoryAmount * memoryState_));
        const auto biased = contoured + (config_.asymmetry * 0.05) + (0.08 * memoryState_);
        const auto normalized = std::tanh(effectiveDrive * biased);
        return normalized / normalization_;
    }

    [[nodiscard]] double memoryState() const {
        return memoryState_;
    }

    [[nodiscard]] double peakMemoryState() const {
        return peakMemoryState_;
    }

private:
    CandidateConfig config_;
    double memoryCoefficient_ {0.0};
    double memoryState_ {0.0};
    double peakMemoryState_ {0.0};
    double normalization_ {1.0};
    ContourFilter contour_;
};

[[nodiscard]] std::vector<double> makeSine(double sampleRate, double frequency, double amplitude, std::size_t numSamples) {
    std::vector<double> signal(numSamples, 0.0);
    const auto phaseIncrement = (2.0 * kPi * frequency) / sampleRate;

    double phase = 0.0;
    for (auto& sample : signal) {
        sample = amplitude * std::sin(phase);
        phase += phaseIncrement;
    }

    return signal;
}

[[nodiscard]] std::vector<double> makeBurst(double sampleRate,
                                            double frequency,
                                            double amplitude,
                                            double burstSeconds,
                                            double silenceSeconds) {
    const auto burstSamples = static_cast<std::size_t>(burstSeconds * sampleRate);
    const auto silenceSamples = static_cast<std::size_t>(silenceSeconds * sampleRate);
    auto signal = makeSine(sampleRate, frequency, amplitude, burstSamples);
    signal.resize(burstSamples + silenceSamples, 0.0);
    return signal;
}

[[nodiscard]] double rms(const std::vector<double>& signal, std::size_t start, std::size_t count) {
    auto sumSquares = 0.0;
    for (std::size_t index = 0; index < count; ++index) {
        const auto sample = signal[start + index];
        sumSquares += sample * sample;
    }

    return std::sqrt(sumSquares / static_cast<double>(count));
}

template <typename Candidate>
[[nodiscard]] std::vector<double> runCandidate(Candidate& candidate, const std::vector<double>& input) {
    std::vector<double> output;
    output.reserve(input.size());

    for (const auto sample : input) {
        output.push_back(candidate.process(sample));
    }

    return output;
}

[[nodiscard]] double measureToneAmplitude(const std::vector<double>& signal, double sampleRate, double frequency) {
    std::complex<double> accumulator {0.0, 0.0};
    const auto phaseIncrement = (-2.0 * kPi * frequency) / sampleRate;

    for (std::size_t index = 0; index < signal.size(); ++index) {
        const auto phase = phaseIncrement * static_cast<double>(index);
        accumulator += signal[index] * std::complex<double>(std::cos(phase), std::sin(phase));
    }

    return (2.0 * std::abs(accumulator)) / static_cast<double>(signal.size());
}

struct ExperimentMetrics {
    double thdPercent {0.0};
    double secondHarmonicDb {0.0};
    double thirdHarmonicDb {0.0};
    double lfDriftDb {0.0};
    double recoveryMs {0.0};
    double nsPerSample {0.0};
};

template <typename Candidate>
[[nodiscard]] ExperimentMetrics evaluateCandidate(Candidate candidate,
                                                  const CandidateConfig& config,
                                                  std::string_view candidateName) {
    candidate.prepare();
    candidate.reset();

    const auto tone = makeSine(config.sampleRate, 1000.0, 0.55, 9600);
    const auto processedTone = runCandidate(candidate, tone);

    const auto fundamental = measureToneAmplitude(processedTone, config.sampleRate, 1000.0);
    const auto second = measureToneAmplitude(processedTone, config.sampleRate, 2000.0);
    const auto third = measureToneAmplitude(processedTone, config.sampleRate, 3000.0);
    const auto fourth = measureToneAmplitude(processedTone, config.sampleRate, 4000.0);
    const auto fifth = measureToneAmplitude(processedTone, config.sampleRate, 5000.0);

    const auto harmonicSum =
        std::sqrt((second * second) + (third * third) + (fourth * fourth) + (fifth * fifth));
    const auto thdPercent = (harmonicSum / std::max(fundamental, 1.0e-12)) * 100.0;

    candidate.reset();
    const auto burst = makeBurst(config.sampleRate, 50.0, 0.75, 0.50, 0.35);
    const auto processedBurst = runCandidate(candidate, burst);

    const auto earlyWindow = static_cast<std::size_t>(0.10 * config.sampleRate);
    const auto lateStart = static_cast<std::size_t>(0.35 * config.sampleRate);
    const auto lateWindow = static_cast<std::size_t>(0.10 * config.sampleRate);
    const auto earlyRms = rms(processedBurst, 0, earlyWindow);
    const auto lateRms = rms(processedBurst, lateStart, lateWindow);
    const auto lfDriftDb = linearToDb(lateRms / std::max(earlyRms, 1.0e-12));

    candidate.reset();
    const auto memoryExercise = makeBurst(config.sampleRate, 50.0, 0.75, 0.50, 1.50);
    for (const auto sample : memoryExercise) {
        static_cast<void>(candidate.process(sample));
    }

    auto recoveryMs = 0.0;
    const auto peakMemory = std::max(std::abs(candidate.memoryState()), 1.0e-12);
    if (peakMemory > 0.0) {
        candidate.reset();
        const auto excitation = makeBurst(config.sampleRate, 50.0, 0.75, 0.50, 0.0);
        for (const auto sample : excitation) {
            static_cast<void>(candidate.process(sample));
        }

        const auto referencePeak = std::max(std::abs(candidate.memoryState()), 1.0e-12);
        std::size_t sampleCount = 0;
        while (sampleCount < static_cast<std::size_t>(2.0 * config.sampleRate)) {
            static_cast<void>(candidate.process(0.0));
            ++sampleCount;
            if (std::abs(candidate.memoryState()) <= (0.1 * referencePeak)) {
                break;
            }
        }

        recoveryMs = (1000.0 * static_cast<double>(sampleCount)) / config.sampleRate;
    }

    candidate.reset();
    auto benchmarkInput = makeSine(config.sampleRate, 997.0, 0.55, static_cast<std::size_t>(config.sampleRate * 4.0));
    volatile double accumulator = 0.0;
    constexpr std::size_t kIterations = 24;

    const auto startTime = std::chrono::steady_clock::now();
    for (std::size_t iteration = 0; iteration < kIterations; ++iteration) {
        candidate.reset();
        for (const auto sample : benchmarkInput) {
            accumulator += candidate.process(sample);
        }
    }
    const auto endTime = std::chrono::steady_clock::now();
    const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
    const auto totalSamples = static_cast<double>(benchmarkInput.size() * kIterations);
    const auto nsPerSample = static_cast<double>(elapsedNs) / totalSamples;

    std::cout << "Candidate " << candidateName << " accumulator guard: " << accumulator << '\n';

    return {
        .thdPercent = thdPercent,
        .secondHarmonicDb = linearToDb(second / std::max(fundamental, 1.0e-12)),
        .thirdHarmonicDb = linearToDb(third / std::max(fundamental, 1.0e-12)),
        .lfDriftDb = lfDriftDb,
        .recoveryMs = recoveryMs,
        .nsPerSample = nsPerSample
    };
}

void printMetrics(std::string_view name, const ExperimentMetrics& metrics) {
    std::cout << name << '\n';
    std::cout << "  THD (%): " << metrics.thdPercent << '\n';
    std::cout << "  2nd harmonic (dBr): " << metrics.secondHarmonicDb << '\n';
    std::cout << "  3rd harmonic (dBr): " << metrics.thirdHarmonicDb << '\n';
    std::cout << "  LF drift over burst (dB): " << metrics.lfDriftDb << '\n';
    std::cout << "  Recovery to 10% memory (ms): " << metrics.recoveryMs << '\n';
    std::cout << "  CPU (ns/sample): " << metrics.nsPerSample << '\n';
}

}  // namespace

int main() {
    std::cout << std::fixed << std::setprecision(3);

    const CandidateConfig config {};

    const auto staticMetrics =
        evaluateCandidate(StaticTransformerCandidate {config}, config, "A");
    const auto memoryMetrics =
        evaluateCandidate(MemoryTransformerCandidate {config}, config, "B");

    std::cout << "\nTransformer hysteresis experiment\n";
    std::cout << "Sample rate: " << config.sampleRate << " Hz\n";
    std::cout << "Drive: " << config.drive << '\n';
    std::cout << "Asymmetry: " << config.asymmetry << '\n';
    std::cout << "Contour amount: " << config.contourAmount << '\n';
    std::cout << "Memory amount: " << config.memoryAmount << "\n\n";

    printMetrics("Candidate A: static contour + saturator", staticMetrics);
    std::cout << '\n';
    printMetrics("Candidate B: contour + bounded memory", memoryMetrics);
    std::cout << '\n';

    const auto cpuRatio = memoryMetrics.nsPerSample / std::max(staticMetrics.nsPerSample, 1.0e-12);
    std::cout << "Relative CPU cost (B/A): " << cpuRatio << "x\n";
    std::cout << "LF drift improvement (B - A): " << (memoryMetrics.lfDriftDb - staticMetrics.lfDriftDb) << " dB\n";

    return 0;
}
