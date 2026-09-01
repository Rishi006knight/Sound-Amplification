#include "HearingLossSimulator.h"
#include <cmath>
#include <algorithm>

namespace Amplify {

HearingLossSimulator::HearingLossSimulator() {
    leftBandLossDb.fill(0.0f);
    rightBandLossDb.fill(0.0f);
    leftBandExpansionRatio.fill(1.0f);
    rightBandExpansionRatio.fill(1.0f);
    prepare(48000.0f);
}

void HearingLossSimulator::prepare(float sr) {
    sampleRate = sr;
    filterBank.prepare(sampleRate);
    reset();
}

void HearingLossSimulator::reset() {
    filterBank.reset();
}

void HearingLossSimulator::updateProfile(const AudiogramData& audiogram) {
    const auto& freqs = FilterBank::getCenterFrequencies();

    for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
        float f = freqs[b];

        float leftLoss = audiogram.getInterpolatedThreshold(Ear::Left, f);
        float rightLoss = audiogram.getInterpolatedThreshold(Ear::Right, f);

        leftBandLossDb[b] = std::clamp(leftLoss, 0.0f, 90.0f);
        rightBandLossDb[b] = std::clamp(rightLoss, 0.0f, 90.0f);

        // Calculate recruitment expansion ratio (soft sounds attenuated much more heavily than loud sounds)
        leftBandExpansionRatio[b] = 1.0f + (leftBandLossDb[b] / 50.0f) * recruitmentFactor;
        rightBandExpansionRatio[b] = 1.0f + (rightBandLossDb[b] / 50.0f) * recruitmentFactor;
    }
}

float HearingLossSimulator::processSimulatedBand(float bandSample, float lossDb, float expansionRatio) {
    if (lossDb <= 2.0f) {
        return bandSample;
    }

    float absSamp = std::abs(bandSample);
    if (absSamp < 1e-7f) return 0.0f;

    // Convert to estimated dB SPL (where 0 dBFS == 100 dB SPL)
    float inDbSPL = AudioBufferUtils::floatToDbSPL(absSamp, 100.0f);

    // If signal level is below user's elevated threshold, heavily attenuate (inaudible)
    float effectiveLossDb = lossDb;
    if (inDbSPL < lossDb + 10.0f) {
        // Below elevated threshold: cutoff sound
        float falloff = (lossDb + 10.0f - inDbSPL) * expansionRatio;
        effectiveLossDb += falloff;
    } else {
        // Above threshold: recruitment causes rapid loudness rise toward UCL
        float rangeAboveThreshold = inDbSPL - lossDb;
        float compressionRelief = std::clamp(rangeAboveThreshold / 50.0f, 0.0f, 0.75f);
        effectiveLossDb -= (lossDb * compressionRelief * recruitmentFactor);
    }

    effectiveLossDb = std::clamp(effectiveLossDb, 0.0f, 95.0f);
    float attenuationFactor = AudioBufferUtils::dbToGain(-effectiveLossDb);

    return bandSample * attenuationFactor;
}

StereoFrame HearingLossSimulator::processFrame(const StereoFrame& inFrame) {
    if (!isEnabled) {
        return inFrame;
    }

    std::array<float, FilterBank::NUM_BANDS> leftBands;
    std::array<float, FilterBank::NUM_BANDS> rightBands;

    filterBank.processSample(inFrame.left, 0, leftBands);
    filterBank.processSample(inFrame.right, 1, rightBands);

    float sumLeft = 0.0f;
    float sumRight = 0.0f;

    for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
        float outL = processSimulatedBand(leftBands[b], leftBandLossDb[b], leftBandExpansionRatio[b]);
        float outR = processSimulatedBand(rightBands[b], rightBandLossDb[b], rightBandExpansionRatio[b]);
        sumLeft += outL;
        sumRight += outR;
    }

    return { sumLeft, sumRight };
}

} // namespace Amplify
