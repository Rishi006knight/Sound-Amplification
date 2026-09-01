#pragma once

#include "Core/AudiogramData.h"
#include "FilterBank.h"
#include "AudioBufferUtils.h"
#include <array>

namespace Amplify {

class HearingLossSimulator {
public:
    HearingLossSimulator();
    ~HearingLossSimulator() = default;

    void prepare(float sampleRate);
    void reset();

    void updateProfile(const AudiogramData& audiogram);

    // Processes a stereo sample through simulated hearing loss
    StereoFrame processFrame(const StereoFrame& inFrame);

    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool getEnabled() const { return isEnabled; }

    void setRecruitmentIntensity(float intensity) { recruitmentFactor = std::clamp(intensity, 0.0f, 1.0f); }
    float getRecruitmentIntensity() const { return recruitmentFactor; }

private:
    float sampleRate{48000.0f};
    bool isEnabled{false};
    float recruitmentFactor{0.7f};

    FilterBank filterBank;

    // Loss attenuation and recruitment curves per band for Left and Right
    std::array<float, FilterBank::NUM_BANDS> leftBandLossDb;
    std::array<float, FilterBank::NUM_BANDS> rightBandLossDb;

    std::array<float, FilterBank::NUM_BANDS> leftBandExpansionRatio;
    std::array<float, FilterBank::NUM_BANDS> rightBandExpansionRatio;

    float processSimulatedBand(float bandSample, float lossDb, float expansionRatio);
};

} // namespace Amplify
