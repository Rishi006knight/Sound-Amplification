#pragma once

#include "AudiogramData.h"
#include <vector>
#include <string>

namespace Amplify {

enum class PrescriptionFormulaType {
    NAL_R,       // National Acoustic Laboratories - Revised (Linear)
    HalfGain,    // 1/2 Gain Rule
    POGO,        // Prescription of Gain and Output (with 250/500 Hz low-cut)
    NAL_NL2      // NAL Non-Linear 2 Multi-Level Target Estimation
};

struct BandTarget {
    float centerFrequencyHz{1000.0f};
    float gainSoftDb{0.0f};     // Gain for 50 dB SPL input
    float gainMediumDb{0.0f};   // Gain for 65 dB SPL input (nominal speech)
    float gainLoudDb{0.0f};     // Gain for 80 dB SPL input
    float compressionRatio{1.0f}; // Dynamic range compression ratio (e.g. 1.0 to 3.5)
    float kneepointDbSPL{50.0f};  // Compression threshold
    float mpoDbSPL{90.0f};        // Maximum Power Output ceiling
};

struct EarProfile {
    Ear ear{Ear::Left};
    std::vector<BandTarget> bandTargets;

    float getNominalGain(float freqHz) const;
    float getSoftGain(float freqHz) const;
    float getLoudGain(float freqHz) const;
    float getCompressionRatio(float freqHz) const;
    float getMPO(float freqHz) const;
};

class HearingProfile {
public:
    HearingProfile();
    ~HearingProfile() = default;

    PrescriptionFormulaType formulaType{PrescriptionFormulaType::NAL_R};

    EarProfile leftEarProfile{Ear::Left};
    EarProfile rightEarProfile{Ear::Right};

    // User fine-tuning master controls
    float masterGainDb{0.0f};      // -12 dB to +12 dB
    float bassTrimDb{0.0f};        // Low frequencies (below 500 Hz)
    float presenceTrimDb{0.0f};    // Speech clarity range (1 kHz to 4 kHz)
    float trebleTrimDb{0.0f};      // High frequencies (above 4 kHz)
    float safetyLimiterCeilingDbSPL{88.0f}; // Absolute safety brickwall threshold (dB SPL)

    // Helper to get effective gain (prescribed + user trims)
    float getEffectiveGain(Ear ear, float frequencyHz, float inputLevelDbSPL = 65.0f) const;

    static std::string getFormulaName(PrescriptionFormulaType type);
};

} // namespace Amplify
