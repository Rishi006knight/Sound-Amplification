#include "Storage/ProfileSerializer.h"
#include "Core/Presets.h"

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

TEST_CASE(ProfileSerializer_AudiogramRoundtrip) {
    AudiogramData original = Presets::createNoiseInducedNotch();
    original.patientName = "Test Subject 01";

    std::string json = ProfileSerializer::serializeAudiogramToJson(original);
    EXPECT_TRUE(!json.empty());

    AudiogramData loaded;
    bool success = ProfileSerializer::deserializeAudiogramFromJson(json, loaded);
    EXPECT_TRUE(success);
    EXPECT_TRUE(loaded.patientName == "Test Subject 01");

    // Check threshold at 4000 Hz notch (65 dB)
    float orig4k = original.getInterpolatedThreshold(Ear::Left, 4000.0f);
    float load4k = loaded.getInterpolatedThreshold(Ear::Left, 4000.0f);
    EXPECT_NEAR(orig4k, load4k, 0.5f);

    return true;
}

TEST_CASE(ProfileSerializer_HearingProfileRoundtrip) {
    HearingProfile original;
    original.formulaType = PrescriptionFormulaType::NAL_NL2;
    original.masterGainDb = 4.5f;
    original.bassTrimDb = -2.0f;
    original.trebleTrimDb = 3.0f;
    original.safetyLimiterCeilingDbSPL = 86.0f;

    std::string json = ProfileSerializer::serializeHearingProfileToJson(original);
    EXPECT_TRUE(!json.empty());

    HearingProfile loaded;
    bool success = ProfileSerializer::deserializeHearingProfileFromJson(json, loaded);
    EXPECT_TRUE(success);
    EXPECT_TRUE(loaded.formulaType == PrescriptionFormulaType::NAL_NL2);
    EXPECT_NEAR(loaded.masterGainDb, 4.5f, 0.1f);
    EXPECT_NEAR(loaded.bassTrimDb, -2.0f, 0.1f);
    EXPECT_NEAR(loaded.trebleTrimDb, 3.0f, 0.1f);
    EXPECT_NEAR(loaded.safetyLimiterCeilingDbSPL, 86.0f, 0.1f);

    return true;
}
