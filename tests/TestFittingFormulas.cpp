#include "Core/AudiogramData.h"
#include "Core/HearingProfile.h"
#include "Core/Presets.h"
#include "Fitting/NALRFormula.h"
#include "Fitting/HalfGainFormula.h"
#include "Fitting/NALNL2Estimator.h"
#include "Fitting/FittingEngine.h"

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

TEST_CASE(NALR_NormalHearing_ZeroGain) {
    AudiogramData normal = Presets::createNormalHearing();
    NALRFormula formula;
    std::vector<float> freqs = {250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f};

    EarProfile profile = formula.calculateEarProfile(normal.getEar(Ear::Left), freqs);
    EXPECT_TRUE(profile.bandTargets.size() == freqs.size());

    // Normal hearing (0-5 dB HL) should produce ~0 dB insertion gain across all bands
    for (const auto& bt : profile.bandTargets) {
        EXPECT_NEAR(bt.gainMediumDb, 0.0f, 2.5f);
    }
    return true;
}

TEST_CASE(NALR_HighFrequencyLoss_CalculatesCorrectSlopes) {
    AudiogramData data = Presets::createMildHighFrequencyLoss();
    NALRFormula formula;
    std::vector<float> freqs = {250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f};

    EarProfile profile = formula.calculateEarProfile(data.getEar(Ear::Left), freqs);

    float gain250 = profile.getNominalGain(250.0f);
    float gain1000 = profile.getNominalGain(1000.0f);
    float gain4000 = profile.getNominalGain(4000.0f);

    // High frequency loss should produce significantly higher gain at 4000 Hz than at 250 Hz
    EXPECT_TRUE(gain4000 > gain1000);
    EXPECT_TRUE(gain1000 >= gain250);
    return true;
}

TEST_CASE(HalfGain_And_POGO_Comparison) {
    AudiogramData data = Presets::createModerateFlatLoss(); // 45 dB loss everywhere
    HalfGainFormula pureHalfGain(false);
    HalfGainFormula pogo(true);
    std::vector<float> freqs = {250.0f, 500.0f, 1000.0f, 2000.0f};

    EarProfile hgProfile = pureHalfGain.calculateEarProfile(data.getEar(Ear::Left), freqs);
    EarProfile pogoProfile = pogo.calculateEarProfile(data.getEar(Ear::Left), freqs);

    // Half gain of 45 dB = 22.5 dB
    EXPECT_NEAR(hgProfile.getNominalGain(1000.0f), 22.5f, 0.5f);

    // POGO cuts 10 dB at 250 Hz and 5 dB at 500 Hz
    EXPECT_NEAR(pogoProfile.getNominalGain(250.0f), 12.5f, 0.5f);
    EXPECT_NEAR(pogoProfile.getNominalGain(500.0f), 17.5f, 0.5f);
    EXPECT_NEAR(pogoProfile.getNominalGain(1000.0f), 22.5f, 0.5f);
    return true;
}

TEST_CASE(NALNL2_NonLinear_CompressionLevels) {
    AudiogramData data = Presets::createModeratePresbycusis();
    NALNL2Estimator nlnl2;
    std::vector<float> freqs = {1000.0f, 2000.0f, 4000.0f};

    EarProfile profile = nlnl2.calculateEarProfile(data.getEar(Ear::Left), freqs);

    for (const auto& bt : profile.bandTargets) {
        // Non-linear rule: Gain for soft inputs (50 dB) must be greater than gain for loud inputs (80 dB)
        EXPECT_TRUE(bt.gainSoftDb > bt.gainMediumDb);
        EXPECT_TRUE(bt.gainMediumDb >= bt.gainLoudDb);
        EXPECT_TRUE(bt.compressionRatio >= 1.0f);
    }
    return true;
}
