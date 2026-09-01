#include "FilterBank.h"
#include <numbers>

namespace Amplify {

static const std::vector<float> CENTER_FREQS = {
    125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 6000.0f, 8000.0f
};

const std::vector<float>& FilterBank::getCenterFrequencies() {
    return CENTER_FREQS;
}

void BiquadFilter::setLowPass(float sr, float cutoffFreq, float q) {
    float w0 = 2.0f * 3.14159265358979323846f * (cutoffFreq / sr);
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float alpha = sinw0 / (2.0f * q);

    float b0 = (1.0f - cosw0) / 2.0f;
    float b1 = 1.0f - cosw0;
    float b2 = (1.0f - cosw0) / 2.0f;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cosw0;
    float a2 = 1.0f - alpha;

    coeffs.b0 = b0 / a0;
    coeffs.b1 = b1 / a0;
    coeffs.b2 = b2 / a0;
    coeffs.a1 = a1 / a0;
    coeffs.a2 = a2 / a0;
}

void BiquadFilter::setHighPass(float sr, float cutoffFreq, float q) {
    float w0 = 2.0f * 3.14159265358979323846f * (cutoffFreq / sr);
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float alpha = sinw0 / (2.0f * q);

    float b0 = (1.0f + cosw0) / 2.0f;
    float b1 = -(1.0f + cosw0);
    float b2 = (1.0f + cosw0) / 2.0f;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cosw0;
    float a2 = 1.0f - alpha;

    coeffs.b0 = b0 / a0;
    coeffs.b1 = b1 / a0;
    coeffs.b2 = b2 / a0;
    coeffs.a1 = a1 / a0;
    coeffs.a2 = a2 / a0;
}

void BiquadFilter::setBandPass(float sr, float centerFreq, float q) {
    float w0 = 2.0f * 3.14159265358979323846f * (centerFreq / sr);
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float alpha = sinw0 / (2.0f * q);

    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cosw0;
    float a2 = 1.0f - alpha;

    coeffs.b0 = b0 / a0;
    coeffs.b1 = b1 / a0;
    coeffs.b2 = b2 / a0;
    coeffs.a1 = a1 / a0;
    coeffs.a2 = a2 / a0;
}

void BiquadFilter::setPeakingEQ(float sr, float centerFreq, float gainDb, float q) {
    float A = std::pow(10.0f, gainDb / 40.0f);
    float w0 = 2.0f * 3.14159265358979323846f * (centerFreq / sr);
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float alpha = sinw0 / (2.0f * q);

    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cosw0;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cosw0;
    float a2 = 1.0f - alpha / A;

    coeffs.b0 = b0 / a0;
    coeffs.b1 = b1 / a0;
    coeffs.b2 = b2 / a0;
    coeffs.a1 = a1 / a0;
    coeffs.a2 = a2 / a0;
}

FilterBank::FilterBank() {
    prepare(48000.0f);
}

void FilterBank::prepare(float sr) {
    sampleRate = sr;

    for (size_t ch = 0; ch < 2; ++ch) {
        // Band 0: Low-pass @ 180 Hz
        channelBands[ch][0].filter1.setLowPass(sampleRate, 180.0f, 0.7071f);
        channelBands[ch][0].filter2.setLowPass(sampleRate, 180.0f, 0.7071f);

        // Band 1: Band-pass @ 250 Hz (Q = 1.4)
        channelBands[ch][1].filter1.setBandPass(sampleRate, 250.0f, 1.414f);
        channelBands[ch][1].filter2.setBandPass(sampleRate, 250.0f, 1.414f);

        // Band 2: Band-pass @ 500 Hz (Q = 1.4)
        channelBands[ch][2].filter1.setBandPass(sampleRate, 500.0f, 1.414f);
        channelBands[ch][2].filter2.setBandPass(sampleRate, 500.0f, 1.414f);

        // Band 3: Band-pass @ 1000 Hz (Q = 1.4)
        channelBands[ch][3].filter1.setBandPass(sampleRate, 1000.0f, 1.414f);
        channelBands[ch][3].filter2.setBandPass(sampleRate, 1000.0f, 1.414f);

        // Band 4: Band-pass @ 2000 Hz (Q = 1.4)
        channelBands[ch][4].filter1.setBandPass(sampleRate, 2000.0f, 1.414f);
        channelBands[ch][4].filter2.setBandPass(sampleRate, 2000.0f, 1.414f);

        // Band 5: Band-pass @ 4000 Hz (Q = 1.4)
        channelBands[ch][5].filter1.setBandPass(sampleRate, 4000.0f, 1.414f);
        channelBands[ch][5].filter2.setBandPass(sampleRate, 4000.0f, 1.414f);

        // Band 6: Band-pass @ 6000 Hz (Q = 1.4)
        channelBands[ch][6].filter1.setBandPass(sampleRate, 6000.0f, 1.414f);
        channelBands[ch][6].filter2.setBandPass(sampleRate, 6000.0f, 1.414f);

        // Band 7: High-pass @ 7000 Hz
        channelBands[ch][7].filter1.setHighPass(sampleRate, 7000.0f, 0.7071f);
        channelBands[ch][7].filter2.setHighPass(sampleRate, 7000.0f, 0.7071f);
    }

    reset();
}

void FilterBank::reset() {
    for (size_t ch = 0; ch < 2; ++ch) {
        for (size_t b = 0; b < NUM_BANDS; ++b) {
            channelBands[ch][b].filter1.reset();
            channelBands[ch][b].filter2.reset();
        }
    }
}

void FilterBank::processSample(float inputSample, size_t channel, std::array<float, NUM_BANDS>& bandOutputs) {
    size_t ch = channel % 2;
    for (size_t b = 0; b < NUM_BANDS; ++b) {
        float f1Out = channelBands[ch][b].filter1.processSample(inputSample);
        bandOutputs[b] = channelBands[ch][b].filter2.processSample(f1Out);
    }
}

} // namespace Amplify
