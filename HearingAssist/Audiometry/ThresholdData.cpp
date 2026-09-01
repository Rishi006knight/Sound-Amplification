#include "ThresholdData.h"

namespace HearingAssist {

static const std::vector<float> STANDARD_FREQS = {
    250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f
};

const std::vector<float>& ThresholdData::getStandardFrequencies() {
    return STANDARD_FREQS;
}

ThresholdData::ThresholdData() {
    for (float f : STANDARD_FREQS) {
        setAirThreshold(Ear::Left, f, 0.0f);
        setAirThreshold(Ear::Right, f, 0.0f);
    }
}

void ThresholdData::setAirThreshold(Ear ear, float frequencyHz, float dbHL, float mcl, float ucl) {
    auto& targetList = (ear == Ear::Left) ? leftEar : rightEar;

    for (auto& pt : targetList) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) {
            pt.thresholdDbHL = std::clamp(dbHL, 0.0f, 120.0f);
            pt.mclDbHL = (mcl >= 0.0f) ? mcl : std::clamp(dbHL + (100.0f - dbHL) * 0.45f, dbHL + 5.0f, 95.0f);
            pt.uclDbHL = (ucl >= 0.0f) ? ucl : std::clamp(dbHL + (100.0f - dbHL) * 0.90f, 85.0f, 115.0f);
            return;
        }
    }

    ThresholdPoint pt;
    pt.frequencyHz = frequencyHz;
    pt.thresholdDbHL = std::clamp(dbHL, 0.0f, 120.0f);
    pt.mclDbHL = (mcl >= 0.0f) ? mcl : std::clamp(dbHL + (100.0f - dbHL) * 0.45f, dbHL + 5.0f, 95.0f);
    pt.uclDbHL = (ucl >= 0.0f) ? ucl : std::clamp(dbHL + (100.0f - dbHL) * 0.90f, 85.0f, 115.0f);
    targetList.push_back(pt);

    std::sort(targetList.begin(), targetList.end(), [](const ThresholdPoint& a, const ThresholdPoint& b) {
        return a.frequencyHz < b.frequencyHz;
    });
}

float ThresholdData::getAirThreshold(Ear ear, float frequencyHz) const {
    const auto& targetList = (ear == Ear::Left) ? leftEar : rightEar;
    for (const auto& pt : targetList) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) return pt.thresholdDbHL;
    }
    return 0.0f;
}

float ThresholdData::getMCL(Ear ear, float frequencyHz) const {
    const auto& targetList = (ear == Ear::Left) ? leftEar : rightEar;
    for (const auto& pt : targetList) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) return pt.mclDbHL;
    }
    return 65.0f;
}

float ThresholdData::getUCL(Ear ear, float frequencyHz) const {
    const auto& targetList = (ear == Ear::Left) ? leftEar : rightEar;
    for (const auto& pt : targetList) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) return pt.uclDbHL;
    }
    return 100.0f;
}

float ThresholdData::getDynamicRange(Ear ear, float frequencyHz) const {
    return std::max(5.0f, getUCL(ear, frequencyHz) - getAirThreshold(ear, frequencyHz));
}

float ThresholdData::interpolateLogFrequency(const std::vector<ThresholdPoint>& pts, float targetFreqHz) {
    if (pts.empty()) return 0.0f;
    if (pts.size() == 1) return pts[0].thresholdDbHL;
    if (targetFreqHz <= pts.front().frequencyHz) return pts.front().thresholdDbHL;
    if (targetFreqHz >= pts.back().frequencyHz) return pts.back().thresholdDbHL;

    float logTarget = std::log2(targetFreqHz);
    for (size_t i = 0; i < pts.size() - 1; ++i) {
        float f1 = pts[i].frequencyHz;
        float f2 = pts[i + 1].frequencyHz;
        if (targetFreqHz >= f1 && targetFreqHz <= f2) {
            float t = (logTarget - std::log2(f1)) / (std::log2(f2) - std::log2(f1));
            return pts[i].thresholdDbHL + t * (pts[i + 1].thresholdDbHL - pts[i].thresholdDbHL);
        }
    }
    return 0.0f;
}

float ThresholdData::getInterpolatedThreshold(Ear ear, float frequencyHz) const {
    return interpolateLogFrequency((ear == Ear::Left) ? leftEar : rightEar, frequencyHz);
}

float ThresholdData::calculatePTA3(Ear ear) const {
    float t500 = getAirThreshold(ear, 500.0f);
    float t1000 = getAirThreshold(ear, 1000.0f);
    float t2000 = getAirThreshold(ear, 2000.0f);
    return (t500 + t1000 + t2000) / 3.0f;
}

LossDegree ThresholdData::getLossDegree(Ear ear) const {
    float pta = calculatePTA3(ear);
    if (pta <= 15.0f) return LossDegree::Normal;
    if (pta <= 25.0f) return LossDegree::Slight;
    if (pta <= 40.0f) return LossDegree::Mild;
    if (pta <= 55.0f) return LossDegree::Moderate;
    if (pta <= 70.0f) return LossDegree::ModeratelySevere;
    if (pta <= 90.0f) return LossDegree::Severe;
    return LossDegree::Profound;
}

std::string ThresholdData::getLossDegreeString(LossDegree degree) {
    switch (degree) {
        case LossDegree::Normal: return "Normal (<=15 dB)";
        case LossDegree::Slight: return "Slight (16-25 dB)";
        case LossDegree::Mild: return "Mild (26-40 dB)";
        case LossDegree::Moderate: return "Moderate (41-55 dB)";
        case LossDegree::ModeratelySevere: return "Moderately Severe (56-70 dB)";
        case LossDegree::Severe: return "Severe (71-90 dB)";
        case LossDegree::Profound: return "Profound (>90 dB)";
        default: return "Unknown";
    }
}

// Clinical Presets
ThresholdData ClinicalPresets::createNormalHearing() {
    ThresholdData data;
    data.profileName = "Normal Hearing (Baseline)";
    data.notes = "All thresholds <= 10 dB HL.";
    for (float f : ThresholdData::getStandardFrequencies()) {
        data.setAirThreshold(Ear::Left, f, 5.0f);
        data.setAirThreshold(Ear::Right, f, 5.0f);
    }
    return data;
}

ThresholdData ClinicalPresets::createMildPresbycusis() {
    ThresholdData data;
    data.profileName = "Mild High-Frequency Presbycusis";
    data.notes = "Age-related high frequency roll-off.";
    std::vector<float> losses = { 10.0f, 15.0f, 20.0f, 35.0f, 45.0f, 55.0f };
    const auto& freqs = ThresholdData::getStandardFrequencies();
    for (size_t i = 0; i < freqs.size(); ++i) {
        data.setAirThreshold(Ear::Left, freqs[i], losses[i]);
        data.setAirThreshold(Ear::Right, freqs[i], losses[i] + 2.0f);
    }
    return data;
}

ThresholdData ClinicalPresets::createModeratePresbycusis() {
    ThresholdData data;
    data.profileName = "Moderate Sloping Presbycusis";
    data.notes = "Steep sensorineural hearing loss in speech clarity range.";
    std::vector<float> losses = { 15.0f, 25.0f, 35.0f, 55.0f, 70.0f, 75.0f };
    const auto& freqs = ThresholdData::getStandardFrequencies();
    for (size_t i = 0; i < freqs.size(); ++i) {
        data.setAirThreshold(Ear::Left, freqs[i], losses[i]);
        data.setAirThreshold(Ear::Right, freqs[i], losses[i]);
    }
    return data;
}

ThresholdData ClinicalPresets::createNoiseInducedNotch() {
    ThresholdData data;
    data.profileName = "Noise-Induced Notch (4 kHz)";
    data.notes = "Classic acoustic trauma dip at 4000 Hz with recovery at 8000 Hz.";
    std::vector<float> losses = { 10.0f, 15.0f, 20.0f, 35.0f, 65.0f, 30.0f };
    const auto& freqs = ThresholdData::getStandardFrequencies();
    for (size_t i = 0; i < freqs.size(); ++i) {
        data.setAirThreshold(Ear::Left, freqs[i], losses[i]);
        data.setAirThreshold(Ear::Right, freqs[i], losses[i] + 5.0f);
    }
    return data;
}

ThresholdData ClinicalPresets::createCookieBite() {
    ThresholdData data;
    data.profileName = "Cookie-Bite (Mid-Frequency Loss)";
    data.notes = "Genetic mid-frequency speech dip.";
    std::vector<float> losses = { 15.0f, 45.0f, 60.0f, 45.0f, 25.0f, 15.0f };
    const auto& freqs = ThresholdData::getStandardFrequencies();
    for (size_t i = 0; i < freqs.size(); ++i) {
        data.setAirThreshold(Ear::Left, freqs[i], losses[i]);
        data.setAirThreshold(Ear::Right, freqs[i], losses[i]);
    }
    return data;
}

ThresholdData ClinicalPresets::createModerateFlatLoss() {
    ThresholdData data;
    data.profileName = "Moderate Flat Loss (45 dB)";
    data.notes = "Even 45 dB loss across all frequencies.";
    for (float f : ThresholdData::getStandardFrequencies()) {
        data.setAirThreshold(Ear::Left, f, 45.0f);
        data.setAirThreshold(Ear::Right, f, 45.0f);
    }
    return data;
}

ThresholdData ClinicalPresets::createAsymmetricLoss() {
    ThresholdData data;
    data.profileName = "Asymmetric Hearing Profile";
    data.notes = "Right ear normal, Left ear moderate high-frequency loss.";
    for (float f : ThresholdData::getStandardFrequencies()) {
        data.setAirThreshold(Ear::Right, f, 10.0f);
    }
    std::vector<float> leftLosses = { 15.0f, 25.0f, 45.0f, 60.0f, 70.0f, 75.0f };
    const auto& freqs = ThresholdData::getStandardFrequencies();
    for (size_t i = 0; i < freqs.size(); ++i) {
        data.setAirThreshold(Ear::Left, freqs[i], leftLosses[i]);
    }
    return data;
}

} // namespace HearingAssist
