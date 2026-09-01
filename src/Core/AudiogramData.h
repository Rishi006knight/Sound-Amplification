#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <map>

namespace Amplify {

enum class Ear {
    Left,
    Right
};

enum class ConductionType {
    Air,
    Bone
};

enum class LossDegree {
    Normal,         // -10 to 15 dB HL (or up to 20/25 in adults)
    Slight,         // 16 to 25 dB HL
    Mild,           // 26 to 40 dB HL
    Moderate,       // 41 to 55 dB HL
    ModeratelySevere,// 56 to 70 dB HL
    Severe,         // 71 to 90 dB HL
    Profound        // 91+ dB HL
};

struct ThresholdPoint {
    float frequencyHz{1000.0f};
    float thresholdDbHL{0.0f};  // Decibels Hearing Level (-10 to 120 dB HL)
    float mclDbHL{65.0f};       // Most Comfortable Level (dB HL)
    float uclDbHL{100.0f};      // Uncomfortable Loudness Level (dB HL)
    bool isNoResponse{false};   // Did the subject respond at max output?
};

struct EarAudiogram {
    Ear ear{Ear::Left};
    std::vector<ThresholdPoint> airConduction;
    std::vector<ThresholdPoint> boneConduction;

    // Speech Audiometry metrics
    float speechReceptionThresholdDb{20.0f}; // SRT in dB HL
    float wordRecognitionScorePercent{96.0f}; // WRS %

    void setAirThreshold(float frequencyHz, float dbHL, float mcl = -1.0f, float ucl = -1.0f);
    float getAirThreshold(float frequencyHz) const;
    float getMCL(float frequencyHz) const;
    float getUCL(float frequencyHz) const;
    float getDynamicRange(float frequencyHz) const;
    
    // Pure Tone Average (500, 1000, 2000 Hz)
    float calculatePTA3() const;
    // High Frequency Pure Tone Average (1000, 2000, 4000 Hz)
    float calculateHFPTA() const;

    LossDegree getOverallLossDegree() const;
    static std::string getLossDegreeName(LossDegree degree);
};

class AudiogramData {
public:
    AudiogramData();
    ~AudiogramData() = default;

    static const std::vector<float>& getStandardFrequencies();

    void initializeDefault();

    EarAudiogram& getEar(Ear ear) { return ear == Ear::Left ? leftEar : rightEar; }
    const EarAudiogram& getEar(Ear ear) const { return ear == Ear::Left ? leftEar : rightEar; }

    void setThreshold(Ear ear, float frequencyHz, float dbHL);
    float getInterpolatedThreshold(Ear ear, float frequencyHz) const;
    float getInterpolatedMCL(Ear ear, float frequencyHz) const;
    float getInterpolatedUCL(Ear ear, float frequencyHz) const;

    // Log-frequency cubic / linear interpolation helper
    static float interpolateLogFrequency(const std::vector<ThresholdPoint>& points, float targetFreqHz, float defaultVal = 0.0f);

    std::string patientId{"ANON-001"};
    std::string patientName{"Default Profile"};
    std::string examinationDate{"2026-09-01"};
    std::string notes{""};

private:
    EarAudiogram leftEar{Ear::Left};
    EarAudiogram rightEar{Ear::Right};
};

} // namespace Amplify
