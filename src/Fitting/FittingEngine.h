#pragma once

#include "IFittingFormula.h"
#include "NALRFormula.h"
#include "HalfGainFormula.h"
#include "NALNL2Estimator.h"
#include "Core/AudiogramData.h"
#include "Core/HearingProfile.h"
#include <memory>
#include <map>

namespace Amplify {

class FittingEngine {
public:
    FittingEngine();
    ~FittingEngine() = default;

    void setFormula(PrescriptionFormulaType formulaType);
    PrescriptionFormulaType getFormulaType() const { return currentFormulaType; }

    // Generates a complete HearingProfile based on current audiogram & formula
    HearingProfile calculateProfile(const AudiogramData& audiogram, const std::vector<float>& filterCenterFrequenciesHz);

    // List available formulas
    std::vector<std::pair<PrescriptionFormulaType, std::string>> getAvailableFormulas() const;

private:
    PrescriptionFormulaType currentFormulaType{PrescriptionFormulaType::NAL_R};
    std::map<PrescriptionFormulaType, std::unique_ptr<IFittingFormula>> formulas;
};

} // namespace Amplify
