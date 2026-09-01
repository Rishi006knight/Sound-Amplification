#include "NALRFormula.h"
#include <cmath>
#include <algorithm>

namespace Amplify {

float NALRFormula::getCorrectionFactorK(float frequencyHz) {
    // Clinical NAL-R k(f) correction factor table
    if (frequencyHz <= 125.0f) return -20.0f;
    if (frequencyHz <= 250.0f) return -17.0f;
    if (frequencyHz <= 500.0f) return -8.0f;
    if (frequencyHz <= 750.0f) return -3.0f;
    if (frequencyHz <= 1000.0f) return +1.0f;
    if (frequencyHz <= 1500.0f) return -1.0f;
    if (frequencyHz <= 2000.0f) return -1.0f;
    if (frequencyHz <= 3000.0f) return -2.0f;
    if (frequencyHz <= 4000.0f) return -2.0f;
    if (frequencyHz <= 6000.0f) return -3.0f;
    return -4.0f; // 8000 Hz and above
}

float NALRFormula::calculateInsertionGain(float frequencyHz, float htlAtFreq, float h3fa) {
    float k = getCorrectionFactorK(frequencyHz);
    float X = 0.05f * h3fa;
    float gain = X + 0.31f * htlAtFreq + k;
    return std::max(0.0f, gain);
}

EarProfile NALRFormula::calculateEarProfile(const EarAudiogram& audiogram, const std::vector<float>& targetFrequenciesHz) {
    EarProfile profile;
    profile.ear = audiogram.ear;
    float h3fa = audiogram.calculatePTA3();

    for (float f : targetFrequenciesHz) {
        float htl = audiogram.getAirThreshold(f);
        float ucl = audiogram.getUCL(f);
        float dynRange = audiogram.getDynamicRange(f);

        float nominalGain = calculateInsertionGain(f, htl, h3fa);

        BandTarget bt;
        bt.centerFrequencyHz = f;
        bt.gainMediumDb = nominalGain;
        bt.gainSoftDb = nominalGain + std::clamp(htl * 0.15f, 0.0f, 6.0f);
        bt.gainLoudDb = std::max(0.0f, nominalGain - std::clamp(htl * 0.15f, 0.0f, 8.0f));

        // Compression ratio to fit normal 65 dB speech dynamic range into user's residual dynamic range
        bt.compressionRatio = std::clamp(65.0f / dynRange, 1.0f, 3.5f);
        bt.kneepointDbSPL = 50.0f;
        bt.mpoDbSPL = std::clamp(ucl, 80.0f, 105.0f);

        profile.bandTargets.push_back(bt);
    }

    return profile;
}

} // namespace Amplify
