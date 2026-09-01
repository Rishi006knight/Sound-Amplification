#pragma once

#include <vector>
#include <array>
#include <cmath>

namespace Amplify {

struct BiquadCoeffs {
    float b0{1.0f}, b1{0.0f}, b2{0.0f};
    float a1{0.0f}, a2{0.0f};
};

class BiquadFilter {
public:
    BiquadFilter() = default;

    void reset() {
        z1 = 0.0f;
        z2 = 0.0f;
    }

    void setCoefficients(const BiquadCoeffs& c) {
        coeffs = c;
    }

    void setLowPass(float sampleRate, float cutoffFreq, float q = 0.7071f);
    void setHighPass(float sampleRate, float cutoffFreq, float q = 0.7071f);
    void setBandPass(float sampleRate, float centerFreq, float q = 1.414f);
    void setPeakingEQ(float sampleRate, float centerFreq, float gainDb, float q = 1.0f);

    inline float processSample(float in) {
        // Direct Form II Transposed structure
        float out = coeffs.b0 * in + z1;
        z1 = coeffs.b1 * in - coeffs.a1 * out + z2;
        z2 = coeffs.b2 * in - coeffs.a2 * out;
        return out;
    }

private:
    BiquadCoeffs coeffs;
    float z1{0.0f};
    float z2{0.0f};
};

class FilterBank {
public:
    static constexpr size_t NUM_BANDS = 8;

    FilterBank();
    ~FilterBank() = default;

    void prepare(float sampleRate);
    void reset();

    static const std::vector<float>& getCenterFrequencies();

    // Splits an input frame into 8 sub-band audio samples
    void processSample(float inputSample, size_t channel, std::array<float, NUM_BANDS>& bandOutputs);

private:
    float sampleRate{48000.0f};

    // 2 audio channels (Left = 0, Right = 1), each having NUM_BANDS filters (cascaded biquads for sharp slopes)
    struct BandProcessor {
        BiquadFilter filter1;
        BiquadFilter filter2;
    };

    std::array<std::array<BandProcessor, NUM_BANDS>, 2> channelBands;
};

} // namespace Amplify
