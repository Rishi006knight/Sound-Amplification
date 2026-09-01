#include "HearingProfile.h"

namespace Amplify {

float EarProfile::getNominalGain(float freqHz) const {
    if (bandTargets.empty()) return 0.0f;
    for (const auto& bt : bandTargets) {
        if (std::abs(bt.centerFrequencyHz - freqHz) < 1.0f) {
            return bt.gainMediumDb;
        }
    }
    // Interpolate
    float logTarget = std::log2(freqHz);
    for (size_t i = 0; i < bandTargets.size() - 1; ++i) {
        float f1 = bandTargets[i].centerFrequencyHz;
        float f2 = bandTargets[i + 1].centerFrequencyHz;
        if (freqHz >= f1 && freqHz <= f2) {
            float t = (logTarget - std::log2(f1)) / (std::log2(f2) - std::log2(f1));
            return bandTargets[i].gainMediumDb + t * (bandTargets[i + 1].gainMediumDb - bandTargets[i].gainMediumDb);
        }
    }
    return bandTargets.front().gainMediumDb;
}

float EarProfile::getSoftGain(float freqHz) const {
    if (bandTargets.empty()) return 0.0f;
    for (const auto& bt : bandTargets) {
        if (std::abs(bt.centerFrequencyHz - freqHz) < 1.0f) {
            return bt.gainSoftDb;
        }
    }
    return getNominalGain(freqHz);
}

float EarProfile::getLoudGain(float freqHz) const {
    if (bandTargets.empty()) return 0.0f;
    for (const auto& bt : bandTargets) {
        if (std::abs(bt.centerFrequencyHz - freqHz) < 1.0f) {
            return bt.gainLoudDb;
        }
    }
    return getNominalGain(freqHz);
}

float EarProfile::getCompressionRatio(float freqHz) const {
    for (const auto& bt : bandTargets) {
        if (std::abs(bt.centerFrequencyHz - freqHz) < 1.0f) {
            return bt.compressionRatio;
        }
    }
    return 1.0f;
}

float EarProfile::getMPO(float freqHz) const {
    for (const auto& bt : bandTargets) {
        if (std::abs(bt.centerFrequencyHz - freqHz) < 1.0f) {
            return bt.mpoDbSPL;
        }
    }
    return 90.0f;
}

HearingProfile::HearingProfile() {
    formulaType = PrescriptionFormulaType::NAL_R;
}

float HearingProfile::getEffectiveGain(Ear ear, float frequencyHz, float inputLevelDbSPL) const {
    const auto& p = (ear == Ear::Left) ? leftEarProfile : rightEarProfile;
    float prescribed = 0.0f;
    if (inputLevelDbSPL <= 55.0f) {
        prescribed = p.getSoftGain(frequencyHz);
    } else if (inputLevelDbSPL >= 75.0f) {
        prescribed = p.getLoudGain(frequencyHz);
    } else {
        float t = (inputLevelDbSPL - 55.0f) / 20.0f;
        prescribed = (1.0f - t) * p.getSoftGain(frequencyHz) + t * p.getLoudGain(frequencyHz);
    }

    // Apply user fine-tuning trims
    float userTrim = masterGainDb;
    if (frequencyHz <= 500.0f) {
        userTrim += bassTrimDb;
    } else if (frequencyHz > 500.0f && frequencyHz < 4000.0f) {
        userTrim += presenceTrimDb;
    } else {
        userTrim += trebleTrimDb;
    }

    return prescribed + userTrim;
}

std::string HearingProfile::getFormulaName(PrescriptionFormulaType type) {
    switch (type) {
        case PrescriptionFormulaType::NAL_R: return "NAL-R (National Acoustic Laboratories)";
        case PrescriptionFormulaType::HalfGain: return "Half-Gain Rule (1/2 Loss)";
        case PrescriptionFormulaType::POGO: return "POGO (Prescription of Gain and Output)";
        case PrescriptionFormulaType::NAL_NL2: return "NAL-NL2 Non-Linear (Multi-Level)";
        default: return "Custom";
    }
}

} // namespace Amplify
