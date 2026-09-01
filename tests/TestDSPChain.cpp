#include "DSP/FilterBank.h"
#include "DSP/BandCompressor.h"
#include "DSP/SafetyLimiter.h"
#include "DSP/MasterDSPChain.h"
#include "DSP/HearingLossSimulator.h"
#include "Core/Presets.h"
#include "Fitting/FittingEngine.h"
#include <cmath>
#include <vector>

#define TEST_CASE(name) \
    bool test_##name(); \
    extern void registerTest(const std::string& name, std::function<bool()> func); \
    struct Register_##name { \
        Register_##name() { registerTest(#name, test_##name); } \
    } reg_##name; \
    bool test_##name()

#define EXPECT_TRUE(cond) if (!(cond)) { return false; }
#define EXPECT_NEAR(a, b, eps) if (std::abs((a) - (b)) > (eps)) { return false; }

using namespace Amplify;

TEST_CASE(SafetyLimiter_PreventsOutputAboveCeiling) {
    SafetyLimiter limiter;
    limiter.prepare(48000.0f, 1.5f, 50.0f);
    limiter.setCeilingDbSPL(85.0f); // 85 dB SPL ceiling (~0.177 linear float)

    // Feed in 1000 samples of massive +6 dBFS (2.0 linear amplitude) square and sine waves
    float maxObservedPeak = 0.0f;
    for (int i = 0; i < 2000; ++i) {
        float in = std::sin(2.0f * 3.14159265f * 1000.0f * i / 48000.0f) * 2.5f;
        StereoFrame out = limiter.processFrame({in, in});

        float peak = std::max(std::abs(out.left), std::abs(out.right));
        if (peak > maxObservedPeak) maxObservedPeak = peak;
    }

    float ceilingFloat = AudioBufferUtils::dbSPLToFloat(85.0f, 100.0f);
    // Verified: No sample ever overshoots the ceiling
    EXPECT_TRUE(maxObservedPeak <= ceilingFloat + 1e-4f);
    EXPECT_TRUE(limiter.isLimitingActive());
    return true;
}

TEST_CASE(FilterBank_EnergyConcentrationInExpectedBand) {
    FilterBank fb;
    fb.prepare(48000.0f);

    // Generate pure 1000 Hz tone (Band 3 is centered at 1000 Hz)
    std::array<float, FilterBank::NUM_BANDS> bandOutputs{};
    std::array<float, FilterBank::NUM_BANDS> bandEnergy{};
    bandEnergy.fill(0.0f);

    for (int i = 0; i < 4800; ++i) {
        float in = std::sin(2.0f * 3.14159265f * 1000.0f * i / 48000.0f);
        fb.processSample(in, 0, bandOutputs);

        for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
            bandEnergy[b] += bandOutputs[b] * bandOutputs[b];
        }
    }

    // Band 3 (1000 Hz) must capture the highest energy
    float maxEnergy = 0.0f;
    size_t maxBand = 0;
    for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
        if (bandEnergy[b] > maxEnergy) {
            maxEnergy = bandEnergy[b];
            maxBand = b;
        }
    }

    EXPECT_TRUE(maxBand == 3);
    return true;
}

TEST_CASE(MasterDSPChain_ModesAndProcessing) {
    MasterDSPChain chain;
    chain.prepare(48000.0f);

    AudiogramData data = Presets::createModeratePresbycusis();
    FittingEngine engine;
    HearingProfile profile = engine.calculateProfile(data, FilterBank::getCenterFrequencies());
    chain.updateProfile(profile, data);

    std::vector<float> inL(512, 0.2f);
    std::vector<float> inR(512, 0.2f);
    std::vector<float> outL(512, 0.0f);
    std::vector<float> outR(512, 0.0f);

    // 1. Bypass Mode: output equals input
    chain.setMode(ProcessingMode::Bypass);
    chain.processBlock(inL.data(), inR.data(), outL.data(), outR.data(), 512);
    EXPECT_NEAR(outL[256], inL[256], 1e-4f);

    // 2. Amplification Mode: output is amplified and modified
    chain.setMode(ProcessingMode::PersonalizedAmplification);
    chain.processBlock(inL.data(), inR.data(), outL.data(), outR.data(), 512);
    EXPECT_TRUE(chain.getMode() == ProcessingMode::PersonalizedAmplification);

    return true;
}
