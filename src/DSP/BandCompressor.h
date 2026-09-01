#pragma once

#include "AudioBufferUtils.h"
#include <cmath>
#include <algorithm>

namespace Amplify {

class BandCompressor {
public:
    BandCompressor();
    ~BandCompressor() = default;

    void prepare(float sampleRate);
    void reset();

    // Configuration parameters
    void setParameters(float linearGainDb, float compressionRatio, float kneepointDbSPL = 50.0f,
                       float attackTimeMs = 10.0f, float releaseTimeMs = 100.0f, float kneeWidthDb = 6.0f);

    float processSample(float inputSample);

    // Current gain reduction in dB for meter/visualizer
    float getCurrentGainReductionDb() const { return currentGainReductionDb; }
    float getEnvelopeDbSPL() const { return currentEnvelopeDbSPL; }

private:
    float sampleRate{48000.0f};

    float linearGainDb{0.0f};
    float compressionRatio{1.0f};
    float kneepointDbSPL{50.0f};
    float kneeWidthDb{6.0f};

    float attackCoeff{0.0f};
    float releaseCoeff{0.0f};

    float envelopeState{0.0f};
    float currentGainReductionDb{0.0f};
    float currentEnvelopeDbSPL{0.0f};

    void updateCoefficients(float attackMs, float releaseMs);
    float calculateStaticGainDb(float inputDbSPL);
};

} // namespace Amplify
