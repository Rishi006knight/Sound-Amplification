#include "AudiogramData.h"

namespace Amplify {

static const std::vector<float> STANDARD_FREQUENCIES = {
    125.0f, 250.0f, 500.0f, 750.0f, 1000.0f, 1500.0f, 2000.0f, 3000.0f, 4000.0f, 6000.0f, 8000.0f
};

const std::vector<float>& AudiogramData::getStandardFrequencies() {
    return STANDARD_FREQUENCIES;
}

void EarAudiogram::setAirThreshold(float frequencyHz, float dbHL, float mcl, float ucl) {
    for (auto& pt : airConduction) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) {
            pt.thresholdDbHL = dbHL;
            if (mcl >= 0.0f) pt.mclDbHL = mcl;
            else pt.mclDbHL = std::clamp(dbHL + (100.0f - dbHL) * 0.45f, dbHL + 5.0f, 95.0f);
            
            if (ucl >= 0.0f) pt.uclDbHL = ucl;
            else pt.uclDbHL = std::clamp(dbHL + (100.0f - dbHL) * 0.90f, 85.0f, 115.0f);
            return;
        }
    }

    ThresholdPoint pt;
    pt.frequencyHz = frequencyHz;
    pt.thresholdDbHL = dbHL;
    pt.mclDbHL = (mcl >= 0.0f) ? mcl : std::clamp(dbHL + (100.0f - dbHL) * 0.45f, dbHL + 5.0f, 95.0f);
    pt.uclDbHL = (ucl >= 0.0f) ? ucl : std::clamp(dbHL + (100.0f - dbHL) * 0.90f, 85.0f, 115.0f);
    airConduction.push_back(pt);

    std::sort(airConduction.begin(), airConduction.end(), [](const ThresholdPoint& a, const ThresholdPoint& b) {
        return a.frequencyHz < b.frequencyHz;
    });
}

float EarAudiogram::getAirThreshold(float frequencyHz) const {
    for (const auto& pt : airConduction) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) {
            return pt.thresholdDbHL;
        }
    }
    return 0.0f;
}

float EarAudiogram::getMCL(float frequencyHz) const {
    for (const auto& pt : airConduction) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) {
            return pt.mclDbHL;
        }
    }
    return 65.0f;
}

float EarAudiogram::getUCL(float frequencyHz) const {
    for (const auto& pt : airConduction) {
        if (std::abs(pt.frequencyHz - frequencyHz) < 1.0f) {
            return pt.uclDbHL;
        }
    }
    return 100.0f;
}

float EarAudiogram::getDynamicRange(float frequencyHz) const {
    return std::max(5.0f, getUCL(frequencyHz) - getAirThreshold(frequencyHz));
}

float EarAudiogram::calculatePTA3() const {
    float t500 = getAirThreshold(500.0f);
    float t1000 = getAirThreshold(1000.0f);
    float t2000 = getAirThreshold(2000.0f);
    return (t500 + t1000 + t2000) / 3.0f;
}

float EarAudiogram::calculateHFPTA() const {
    float t1000 = getAirThreshold(1000.0f);
    float t2000 = getAirThreshold(2000.0f);
    float t4000 = getAirThreshold(4000.0f);
    return (t1000 + t2000 + t4000) / 3.0f;
}

LossDegree EarAudiogram::getOverallLossDegree() const {
    float pta = calculatePTA3();
    if (pta <= 15.0f) return LossDegree::Normal;
    if (pta <= 25.0f) return LossDegree::Slight;
    if (pta <= 40.0f) return LossDegree::Mild;
    if (pta <= 55.0f) return LossDegree::Moderate;
    if (pta <= 70.0f) return LossDegree::ModeratelySevere;
    if (pta <= 90.0f) return LossDegree::Severe;
    return LossDegree::Profound;
}

std::string EarAudiogram::getLossDegreeName(LossDegree degree) {
    switch (degree) {
        case LossDegree::Normal: return "Normal Hearing (<= 15 dB)";
        case LossDegree::Slight: return "Slight Loss (16-25 dB)";
        case LossDegree::Mild: return "Mild Hearing Loss (26-40 dB)";
        case LossDegree::Moderate: return "Moderate Hearing Loss (41-55 dB)";
        case LossDegree::ModeratelySevere: return "Moderately Severe (56-70 dB)";
        case LossDegree::Severe: return "Severe Hearing Loss (71-90 dB)";
        case LossDegree::Profound: return "Profound Hearing Loss (> 90 dB)";
        default: return "Unknown";
    }
}

AudiogramData::AudiogramData() {
    initializeDefault();
}

void AudiogramData::initializeDefault() {
    leftEar.airConduction.clear();
    rightEar.airConduction.clear();

    // Default: normal hearing curve (0 dB HL across all standard octaves)
    for (float f : getStandardFrequencies()) {
        leftEar.setAirThreshold(f, 0.0f);
        rightEar.setAirThreshold(f, 0.0f);
    }
}

void AudiogramData::setThreshold(Ear ear, float frequencyHz, float dbHL) {
    getEar(ear).setAirThreshold(frequencyHz, dbHL);
}

float AudiogramData::interpolateLogFrequency(const std::vector<ThresholdPoint>& points, float targetFreqHz, float defaultVal) {
    if (points.empty()) return defaultVal;
    if (points.size() == 1) return points[0].thresholdDbHL;

    if (targetFreqHz <= points.front().frequencyHz) return points.front().thresholdDbHL;
    if (targetFreqHz >= points.back().frequencyHz) return points.back().thresholdDbHL;

    // Log-frequency linear interpolation
    float logTarget = std::log2(targetFreqHz);

    for (size_t i = 0; i < points.size() - 1; ++i) {
        float f1 = points[i].frequencyHz;
        float f2 = points[i + 1].frequencyHz;

        if (targetFreqHz >= f1 && targetFreqHz <= f2) {
            float logF1 = std::log2(f1);
            float logF2 = std::log2(f2);
            float t = (logTarget - logF1) / (logF2 - logF1);

            return points[i].thresholdDbHL + t * (points[i + 1].thresholdDbHL - points[i].thresholdDbHL);
        }
    }
    return defaultVal;
}

float AudiogramData::getInterpolatedThreshold(Ear ear, float frequencyHz) const {
    const auto& pts = getEar(ear).airConduction;
    return interpolateLogFrequency(pts, frequencyHz, 0.0f);
}

float AudiogramData::getInterpolatedMCL(Ear ear, float frequencyHz) const {
    const auto& pts = getEar(ear).airConduction;
    std::vector<ThresholdPoint> mclPts;
    for (const auto& p : pts) {
        ThresholdPoint tp;
        tp.frequencyHz = p.frequencyHz;
        tp.thresholdDbHL = p.mclDbHL;
        mclPts.push_back(tp);
    }
    return interpolateLogFrequency(mclPts, frequencyHz, 65.0f);
}

float AudiogramData::getInterpolatedUCL(Ear ear, float frequencyHz) const {
    const auto& pts = getEar(ear).airConduction;
    std::vector<ThresholdPoint> uclPts;
    for (const auto& p : pts) {
        ThresholdPoint tp;
        tp.frequencyHz = p.frequencyHz;
        tp.thresholdDbHL = p.uclDbHL;
        uclPts.push_back(tp);
    }
    return interpolateLogFrequency(uclPts, frequencyHz, 100.0f);
}

} // namespace Amplify
