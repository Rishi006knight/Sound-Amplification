#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace Amplify {

struct StereoFrame {
    float left{0.0f};
    float right{0.0f};
};

class AudioBufferUtils {
public:
    static inline float dbToGain(float db) {
        return std::pow(10.0f, db / 20.0f);
    }

    static inline float gainToDb(float gain) {
        return (gain > 1e-6f) ? (20.0f * std::log10(gain)) : -120.0f;
    }

    static inline float clamp(float value, float minVal, float maxVal) {
        return std::clamp(value, minVal, maxVal);
    }

    // Convert float sample [-1.0, 1.0] to rough acoustic dB SPL equivalent (assuming 0 dBFS ~= 100 dB SPL peak in standard headphones)
    static inline float floatToDbSPL(float peakSample, float maxDbSPLAtZeroDBFS = 100.0f) {
        float dbFS = gainToDb(std::abs(peakSample));
        return maxDbSPLAtZeroDBFS + dbFS;
    }

    static inline float dbSPLToFloat(float dbSPL, float maxDbSPLAtZeroDBFS = 100.0f) {
        float dbFS = dbSPL - maxDbSPLAtZeroDBFS;
        return dbToGain(dbFS);
    }
};

} // namespace Amplify
