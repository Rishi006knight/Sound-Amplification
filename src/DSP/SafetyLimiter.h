#pragma once

#include "AudioBufferUtils.h"
#include <vector>
#include <array>
#include <cmath>

namespace Amplify {

class SafetyLimiter {
public:
    SafetyLimiter();
    ~SafetyLimiter() = default;

    void prepare(float sampleRate, float lookaheadMs = 1.5f, float releaseMs = 50.0f);
    void reset();

    // Sets ceiling in equivalent dB SPL (and internal linear limit)
    void setCeilingDbSPL(float ceilingDbSPL);
    float getCeilingDbSPL() const { return currentCeilingDbSPL; }

    // Stereo processing (in-place or frame-by-frame)
    StereoFrame processFrame(const StereoFrame& inFrame);

    float getCurrentLimitingReductionDb() const { return currentReductionDb; }
    bool isLimitingActive() const { return currentReductionDb > 0.1f; }

private:
    float sampleRate{48000.0f};
    float currentCeilingDbSPL{88.0f};
    float linearCeiling{0.5f}; // Clamped linear float threshold

    size_t lookaheadSamples{64};
    std::vector<StereoFrame> lookaheadBuffer;
    size_t writeIndex{0};

    float attackCoeff{0.0f};
    float releaseCoeff{0.0f};
    float smoothedGain{1.0f};
    float currentReductionDb{0.0f};

    void updateCoefficients(float attackMs, float releaseMs);
};

} // namespace Amplify
