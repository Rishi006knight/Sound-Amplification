#include "SafetyLimiter.h"
#include <algorithm>

namespace Amplify {

SafetyLimiter::SafetyLimiter() {
    prepare(48000.0f, 1.5f, 50.0f);
}

void SafetyLimiter::prepare(float sr, float lookaheadMs, float releaseMs) {
    sampleRate = sr;
    lookaheadSamples = std::max<size_t>(1, static_cast<size_t>(sr * (lookaheadMs * 0.001f)));
    lookaheadBuffer.assign(lookaheadSamples, {0.0f, 0.0f});
    writeIndex = 0;

    // Very fast attack (~0.5 ms) to smoothly preempt peaks
    updateCoefficients(0.5f, releaseMs);
    setCeilingDbSPL(currentCeilingDbSPL);
    reset();
}

void SafetyLimiter::reset() {
    std::fill(lookaheadBuffer.begin(), lookaheadBuffer.end(), StereoFrame{0.0f, 0.0f});
    writeIndex = 0;
    smoothedGain = 1.0f;
    currentReductionDb = 0.0f;
}

void SafetyLimiter::updateCoefficients(float attackMs, float releaseMs) {
    attackCoeff = std::exp(-1.0f / (sampleRate * (std::max(0.05f, attackMs) * 0.001f)));
    releaseCoeff = std::exp(-1.0f / (sampleRate * (std::max(1.0f, releaseMs) * 0.001f)));
}

void SafetyLimiter::setCeilingDbSPL(float ceilingDbSPL) {
    currentCeilingDbSPL = std::clamp(ceilingDbSPL, 70.0f, 95.0f);
    // Convert 88 dB SPL to linear float (where 100 dB SPL == 1.0 float peak)
    // dbFS = ceilingDbSPL - 100
    linearCeiling = AudioBufferUtils::dbSPLToFloat(currentCeilingDbSPL, 100.0f);
    linearCeiling = std::clamp(linearCeiling, 0.05f, 0.98f);
}

StereoFrame SafetyLimiter::processFrame(const StereoFrame& inFrame) {
    // 1. Store input frame into circular lookahead buffer
    size_t readIndex = (writeIndex + 1) % lookaheadSamples;
    StereoFrame delayedFrame = lookaheadBuffer[readIndex];
    lookaheadBuffer[writeIndex] = inFrame;
    writeIndex = (writeIndex + 1) % lookaheadSamples;

    // 2. Measure peak of incoming frame
    float peak = std::max(std::abs(inFrame.left), std::abs(inFrame.right));

    // 3. Compute target gain to avoid exceeding ceiling
    float targetGain = 1.0f;
    if (peak > linearCeiling && linearCeiling > 1e-5f) {
        targetGain = linearCeiling / peak;
    }

    // 4. Smooth gain factor with fast attack, gradual release
    if (targetGain < smoothedGain) {
        smoothedGain = attackCoeff * smoothedGain + (1.0f - attackCoeff) * targetGain;
    } else {
        smoothedGain = releaseCoeff * smoothedGain + (1.0f - releaseCoeff) * targetGain;
    }

    currentReductionDb = (smoothedGain < 0.999f) ? AudioBufferUtils::gainToDb(1.0f / smoothedGain) : 0.0f;

    // 5. Apply smoothed gain to delayed frame
    StereoFrame outFrame;
    outFrame.left = delayedFrame.left * smoothedGain;
    outFrame.right = delayedFrame.right * smoothedGain;

    // Final safety clamp to guarantee 0 overshoot
    outFrame.left = std::clamp(outFrame.left, -linearCeiling, linearCeiling);
    outFrame.right = std::clamp(outFrame.right, -linearCeiling, linearCeiling);

    return outFrame;
}

} // namespace Amplify
