#pragma once

#include "IFittingFormula.h"

namespace Amplify {

class HalfGainFormula : public IFittingFormula {
public:
    explicit HalfGainFormula(bool usePOGOCorrections = false);
    ~HalfGainFormula() override = default;

    PrescriptionFormulaType getType() const override {
        return usePOGO ? PrescriptionFormulaType::POGO : PrescriptionFormulaType::HalfGain;
    }
    
    std::string getName() const override {
        return usePOGO ? "POGO (Prescription of Gain and Output)" : "Half-Gain Rule (1/2 Loss)";
    }
    
    std::string getDescription() const override {
        return usePOGO 
            ? "Half-gain rule with 250 Hz and 500 Hz low-frequency insertion cut to reduce upward spread of masking."
            : "Direct half-gain target: provides insertion gain equal to 50% of hearing threshold loss.";
    }

    EarProfile calculateEarProfile(const EarAudiogram& audiogram, const std::vector<float>& targetFrequenciesHz) override;

private:
    bool usePOGO{false};
};

} // namespace Amplify
