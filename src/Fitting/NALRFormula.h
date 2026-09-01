#pragma once

#include "IFittingFormula.h"

namespace Amplify {

class NALRFormula : public IFittingFormula {
public:
    NALRFormula() = default;
    ~NALRFormula() override = default;

    PrescriptionFormulaType getType() const override { return PrescriptionFormulaType::NAL_R; }
    std::string getName() const override { return "NAL-R (National Acoustic Laboratories)"; }
    std::string getDescription() const override {
        return "Standard linear prescription formula maximizing speech intelligibility across comfortable listening levels.";
    }

    EarProfile calculateEarProfile(const EarAudiogram& audiogram, const std::vector<float>& targetFrequenciesHz) override;

    // Direct insertion gain calculation for a single frequency
    static float calculateInsertionGain(float frequencyHz, float htlAtFreq, float h3fa);
    static float getCorrectionFactorK(float frequencyHz);
};

} // namespace Amplify
