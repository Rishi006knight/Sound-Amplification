#pragma once

#include "Core/HearingProfile.h"
#include "Core/AudiogramData.h"
#include "FilterBank.h"
#include "BandCompressor.h"
#include "SafetyLimiter.h"
#include "HearingLossSimulator.h"
#include "AudioBufferUtils.h"
#include <array>
#include <mutex>
#include <atomic>

namespace Amplify {

enum class ProcessingMode {
    Bypass,                     // Mode A: Clean direct audio
    HearingLossSimulation,      // Mode B: Simulated hearing loss
    PersonalizedAmplification   // Mode C: Full personalized DSP compensation
};

struct MeterData {
    float inputPeakDb{-100.0f};
    float inputRmsDb{-100.0f};
    float outputPeakDb{-100.0f};
    float outputRmsDb{-100.0f};
    float limiterGainReductionDb{0.0f};
    std::array<float, FilterBank::NUM_BANDS> leftBandReductionDb{};
    std::array<float, FilterBank::NUM_BANDS> rightBandReductionDb{};
};

class MasterDSPChain {
public:
    MasterDSPChain();
    ~MasterDSPChain() = default;

    void prepare(float sampleRate);
    void reset();

    void setMode(ProcessingMode mode);
    ProcessingMode getMode() const { return currentMode.load(); }

    void updateProfile(const HearingProfile& profile, const AudiogramData& audiogram);

    // Audio block processing (stereo interleaved or separate channels)
    void processBlock(const float* inputLeft, const float* inputRight,
                      float* outputLeft, float* outputRight, size_t numSamples);

    void processInterleaved(const float* inputInterleaved, float* outputInterleaved, size_t numFrames);

    // Metering telemetry for GUI
    MeterData getMeterData();

private:
    float sampleRate{48000.0f};
    std::atomic<ProcessingMode> currentMode{ProcessingMode::PersonalizedAmplification};

    // Filter banks for Left & Right
    FilterBank filterBank;

    // 8 WDRC band compressors for Left and 8 for Right
    std::array<BandCompressor, FilterBank::NUM_BANDS> leftCompressors;
    std::array<BandCompressor, FilterBank::NUM_BANDS> rightCompressors;

    // Additional post-shaping EQ filters for Bass, Presence, Treble trims
    std::array<BiquadFilter, 2> bassEQ;
    std::array<BiquadFilter, 2> presenceEQ;
    std::array<BiquadFilter, 2> trebleEQ;

    // Safety Limiter
    SafetyLimiter safetyLimiter;

    // Hearing Loss Simulator
    HearingLossSimulator lossSimulator;

    // Metering trackers
    std::atomic<float> inPeak{0.0f};
    std::atomic<float> inRmsSum{0.0f};
    std::atomic<float> outPeak{0.0f};
    std::atomic<float> outRmsSum{0.0f};
    size_t meterSampleCount{0};

    std::mutex profileMutex;
};

} // namespace Amplify
