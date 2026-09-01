#include "NALNL2Estimator.h"
#include "NALRFormula.h"
#include <cmath>
#include <algorithm>

namespace Amplify {

float NALNL2Estimator::estimateTargetGain(float frequencyHz, float htlAtFreq, float h3fa, float inputLevelDbSPL) {
    if (htlAtFreq <= 10.0f) return 0.0f;

    // Linear baseline at 65 dB SPL input
    float k = NALRFormula::getCorrectionFactorK(frequencyHz);
    float baseGain65 = 0.05f * h3fa + 0.31f * htlAtFreq + k;
    baseGain65 = std::max(0.0f, baseGain65);

    // Non-linear slope adjustment according to input level (compression)
    // For 50 dB SPL input (soft): more gain (expansion of speech cues)
    // For 80 dB SPL input (loud): less gain (protect acoustic discomfort)
    float slopeFactor = std::clamp(htlAtFreq / 80.0f, 0.2f, 0.6f);
    float deltaLevel = inputLevelDbSPL - 65.0f;

    float targetGain = baseGain65 - (deltaLevel * slopeFactor);
    return std::max(0.0f, targetGain);
}

EarProfile NALNL2Estimator::calculateEarProfile(const EarAudiogram& audiogram, const std::vector<float>& targetFrequenciesHz) {
    EarProfile profile;
    profile.ear = audiogram.ear;
    float h3fa = audiogram.calculatePTA3();

    for (float f : targetFrequenciesHz) {
        float htl = audiogram.getAirThreshold(f);
        float ucl = audiogram.getUCL(f);
        float dynRange = audiogram.getDynamicRange(f);

        float gain50 = estimateTargetGain(f, htl, h3fa, 50.0f);
        float gain65 = estimateTargetGain(f, htl, h3fa, 65.0f);
        float gain80 = estimateTargetGain(f, htl, h3fa, 80.0f);

        // Compression ratio calculated directly from output level delta
        // Input delta: 80 - 50 = 30 dB
        // Output delta: (80 + gain80) - (50 + gain50)
        float out50 = 50.0f + gain50;
        float out80 = 80.0f + gain80;
        float outDelta = std::max(5.0f, out80 - out50);
        float cr = std::clamp(30.0f / outDelta, 1.0f, 3.5f);

        BandTarget bt;
        bt.centerFrequencyHz = f;
        bt.gainSoftDb = gain50;
        bt.gainMediumDb = gain65;
        bt.gainLoudDb = gain80;
        bt.compressionRatio = cr;
        bt.kneepointDbSPL = 48.0f;
        bt.mpoDbSPL = std::clamp(ucl, 82.0f, 105.0f);

        profile.bandTargets.push_back(bt);
    }

    return profile;
}

} // namespace Amplify
