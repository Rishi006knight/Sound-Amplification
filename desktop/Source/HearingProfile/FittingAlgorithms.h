#pragma once

#include <array>
#include <vector>
#include <string>
#include "Audiometry/ThresholdData.h"

namespace HearingAssist {

enum class PrescriptionFormula {
    HalfGain,    // 1/2 Loss Rule
    POGO,        // Prescription of Gain & Output (Low-cut speech target)
    NAL_R,       // National Acoustic Laboratories - Revised (Speech intelligibility)
    NAL_NL2      // NAL Non-Linear Multi-Level Target
};

struct FittingTargets {
    std::array<float, 6> leftGainsDb{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::array<float, 6> rightGainsDb{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::array<float, 6> compressionRatios{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
};

class FittingAlgorithms {
public:
    static FittingTargets calculateTargets(const ThresholdData& audiogram,
                                          PrescriptionFormula formula,
                                          bool isSimulation,
                                          float masterGainDb = 0.0f,
                                          float bassTrimDb = 0.0f,
                                          float trebleTrimDb = 0.0f);

    static std::string getFormulaName(PrescriptionFormula formula);
    static std::vector<std::pair<PrescriptionFormula, std::string>> getAvailableFormulas();

private:
    static float calculateNALRInsertionGain(float freqHz, float htl, float pta3);
    static float getNALRKFactor(float freqHz);
};

} // namespace HearingAssist
