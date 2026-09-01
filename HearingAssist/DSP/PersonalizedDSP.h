#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>

namespace HearingAssist {

enum class ProcessingMode {
    Bypass = 0,                 // Mode A: Clean direct audio
    HearingLossSimulation = 1,  // Mode B: Simulated hearing impairment
    PersonalizedAmplification = 2 // Mode C: Full personalized audiological correction
};

struct MeteringInfo {
    float inputPeakDb{-100.0f};
    float inputRmsDb{-100.0f};
    float outputPeakDb{-100.0f};
    float outputRmsDb{-100.0f};
    bool isLimitingActive{false};
};

class PersonalizedDSP
{
public:
    PersonalizedDSP();
    ~PersonalizedDSP();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void updateProfile(const std::array<float, 6>& leftGainsDb,
                       const std::array<float, 6>& rightGainsDb);

    void setMode(ProcessingMode mode) { currentMode.store(mode); }
    ProcessingMode getMode() const { return currentMode.load(); }

    void setLimiterCeiling(float ceilingDbFS);

    void process(juce::AudioBuffer<float>& buffer);

    MeteringInfo getMeteringInfo() const;

private:
    void updateFilterCoefficients();

    double currentSampleRate{48000.0};
    std::atomic<ProcessingMode> currentMode{ProcessingMode::PersonalizedAmplification};

    static constexpr int numBands = 6;
    const std::array<float, numBands> frequencies = { 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f };
    const float qFactor = 1.414f;

    std::array<float, numBands> leftGainsDb { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    std::array<float, numBands> rightGainsDb { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    std::array<juce::dsp::IIR::Filter<float>, numBands> leftFilters;
    std::array<juce::dsp::IIR::Filter<float>, numBands> rightFilters;

    // Safety Limiter / Compressor for UCL control
    juce::dsp::Compressor<float> safetyCompressor;
    juce::dsp::Limiter<float> outputLimiter;

    // Real-time atomic meters
    std::atomic<float> inPeak{-100.0f};
    std::atomic<float> inRms{-100.0f};
    std::atomic<float> outPeak{-100.0f};
    std::atomic<float> outRms{-100.0f};
    std::atomic<bool> limiterActive{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PersonalizedDSP)
};

} // namespace HearingAssist
