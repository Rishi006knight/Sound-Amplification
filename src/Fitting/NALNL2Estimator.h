#pragma once

#include "IFittingFormula.h"

namespace Amplify {

class NALNL2Estimator : public IFittingFormula {
public:
    NALNL2Estimator() = default;
    ~NALNL2Estimator() override = default;

    PrescriptionFormulaType getType() const override { return PrescriptionFormulaType::NAL_NL2; }
    std::string getName() const override { return "NAL-NL2 (Non-Linear Multi-Level)"; }
    std::string getDescription() const override {
        return "Modern empirical non-linear prescription calculating level-dependent target gains (50, 65, 80 dB SPL) for dynamic range compression.";
    }

    EarProfile calculateEarProfile(const EarAudiogram& audiogram, const std::vector<float>& targetFrequenciesHz) override;

    static float estimateTargetGain(float frequencyHz, float htlAtFreq, float h3fa, float inputLevelDbSPL);
};

} // namespace Amplify
