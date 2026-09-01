#pragma once

#include "AudioBufferUtils.h"
#include <vector>
#include <string>
#include <atomic>
#include <random>

namespace Amplify {

enum class GeneratorType {
    Silence,
    PureTone,
    PinkNoise,
    SineSweep,
    SpeechSampleDemo
};

class WavePlayer {
public:
    WavePlayer();
    ~WavePlayer() = default;

    void prepare(float sampleRate);

    void setGeneratorType(GeneratorType type);
    GeneratorType getGeneratorType() const { return currentType.load(); }

    void setPureTone(float frequencyHz, float levelDbSPL = 65.0f, bool playLeft = true, bool playRight = true);
    void triggerTonePulse(float frequencyHz, float levelDbSPL, float durationMs = 1000.0f, bool playLeft = true, bool playRight = true);

    void fillBuffer(float* interleavedDst, size_t numFrames, unsigned int channels);

private:
    float sampleRate{48000.0f};
    std::atomic<GeneratorType> currentType{GeneratorType::Silence};

    float toneFreq{1000.0f};
    float toneLevelLinear{0.1f};
    float tonePhase{0.0f};
    bool toneLeftEar{true};
    bool toneRightEar{true};

    std::atomic<int> remainingPulseSamples{0};

    // Pink noise filter states
    float b0{0.0f}, b1{0.0f}, b2{0.0f}, b3{0.0f}, b4{0.0f}, b5{0.0f}, b6{0.0f};
    std::mt19937 rng;
    std::uniform_real_distribution<float> whiteDist{-1.0f, 1.0f};

    // Speech demo synthesizer harmonics
    float speechFormantPhase1{0.0f};
    float speechFormantPhase2{0.0f};
    float speechFundPhase{0.0f};
};

} // namespace Amplify
