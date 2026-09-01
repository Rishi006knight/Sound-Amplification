#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace HearingAssist {

enum class Ear {
    Left,
    Right
};

enum class LossDegree {
    Normal,          // <= 15 dB HL
    Slight,          // 16-25 dB HL
    Mild,            // 26-40 dB HL
    Moderate,        // 41-55 dB HL
    ModeratelySevere,// 56-70 dB HL
    Severe,          // 71-90 dB HL
    Profound         // > 90 dB HL
};

struct ThresholdPoint {
    float frequencyHz{1000.0f};
    float thresholdDbHL{0.0f};
    float mclDbHL{65.0f};
    float uclDbHL{100.0f};
};

class ThresholdData {
public:
    static constexpr int NUM_STANDARD_OCTAVES = 6;
    static const std::vector<float>& getStandardFrequencies();

    ThresholdData();
    ~ThresholdData() = default;

    void setAirThreshold(Ear ear, float frequencyHz, float dbHL, float mcl = -1.0f, float ucl = -1.0f);
    float getAirThreshold(Ear ear, float frequencyHz) const;
    float getMCL(Ear ear, float frequencyHz) const;
    float getUCL(Ear ear, float frequencyHz) const;
    float getDynamicRange(Ear ear, float frequencyHz) const;

    float getInterpolatedThreshold(Ear ear, float frequencyHz) const;

    float calculatePTA3(Ear ear) const;
    LossDegree getLossDegree(Ear ear) const;
    static std::string getLossDegreeString(LossDegree degree);

    std::string profileName{"Default Profile"};
    std::string notes{""};

private:
    std::vector<ThresholdPoint> leftEar;
    std::vector<ThresholdPoint> rightEar;

    static float interpolateLogFrequency(const std::vector<ThresholdPoint>& pts, float targetFreqHz);
};

class ClinicalPresets {
public:
    static ThresholdData createNormalHearing();
    static ThresholdData createMildPresbycusis();
    static ThresholdData createModeratePresbycusis();
    static ThresholdData createNoiseInducedNotch();
    static ThresholdData createCookieBite();
    static ThresholdData createModerateFlatLoss();
    static ThresholdData createAsymmetricLoss();
};

} // namespace HearingAssist
