#include "BandCompressor.h"

namespace Amplify {

BandCompressor::BandCompressor() {
    prepare(48000.0f);
}

void BandCompressor::prepare(float sr) {
    sampleRate = sr;
    updateCoefficients(10.0f, 100.0f);
    reset();
}

void BandCompressor::reset() {
    envelopeState = 0.0f;
    currentGainReductionDb = 0.0f;
    currentEnvelopeDbSPL = 0.0f;
}

void BandCompressor::updateCoefficients(float attackMs, float releaseMs) {
    attackCoeff = std::exp(-1.0f / (sampleRate * (std::max(0.1f, attackMs) * 0.001f)));
    releaseCoeff = std::exp(-1.0f / (sampleRate * (std::max(1.0f, releaseMs) * 0.001f)));
}

void BandCompressor::setParameters(float gainDb, float ratio, float kneepoint,
                                   float attackMs, float releaseMs, float kneeWidth) {
    linearGainDb = gainDb;
    compressionRatio = std::max(1.0f, ratio);
    kneepointDbSPL = kneepoint;
    kneeWidthDb = std::max(0.1f, kneeWidth);
    updateCoefficients(attackMs, releaseMs);
}

float BandCompressor::calculateStaticGainDb(float inDbSPL) {
    if (compressionRatio <= 1.001f) {
        return linearGainDb;
    }

    float T = kneepointDbSPL;
    float W = kneeWidthDb;
    float R = compressionRatio;

    float gainChangeDb = 0.0f;

    if (inDbSPL <= T - W / 2.0f) {
        // Below knee: linear amplification with full prescribed target gain
        gainChangeDb = 0.0f;
    } else if (inDbSPL > T - W / 2.0f && inDbSPL < T + W / 2.0f) {
        // In the soft knee quadratic curve
        float delta = inDbSPL - T + W / 2.0f;
        gainChangeDb = (1.0f / R - 1.0f) * (delta * delta) / (2.0f * W);
    } else {
        // Above knee: compression
        gainChangeDb = (1.0f / R - 1.0f) * (inDbSPL - T);
    }

    return linearGainDb + gainChangeDb;
}

float BandCompressor::processSample(float inputSample) {
    float absIn = std::abs(inputSample);

    // Envelope detector (branching smooth follower)
    if (absIn > envelopeState) {
        envelopeState = attackCoeff * envelopeState + (1.0f - attackCoeff) * absIn;
    } else {
        envelopeState = releaseCoeff * envelopeState + (1.0f - releaseCoeff) * absIn;
    }

    currentEnvelopeDbSPL = AudioBufferUtils::floatToDbSPL(envelopeState);
    float desiredGainDb = calculateStaticGainDb(currentEnvelopeDbSPL);

    currentGainReductionDb = linearGainDb - desiredGainDb;
    float linearFactor = AudioBufferUtils::dbToGain(desiredGainDb);

    return inputSample * linearFactor;
}

} // namespace Amplify
