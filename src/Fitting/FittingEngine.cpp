#include "FittingEngine.h"

namespace Amplify {

FittingEngine::FittingEngine() {
    formulas[PrescriptionFormulaType::NAL_R] = std::make_unique<NALRFormula>();
    formulas[PrescriptionFormulaType::HalfGain] = std::make_unique<HalfGainFormula>(false);
    formulas[PrescriptionFormulaType::POGO] = std::make_unique<HalfGainFormula>(true);
    formulas[PrescriptionFormulaType::NAL_NL2] = std::make_unique<NALNL2Estimator>();
}

void FittingEngine::setFormula(PrescriptionFormulaType formulaType) {
    currentFormulaType = formulaType;
}

HearingProfile FittingEngine::calculateProfile(const AudiogramData& audiogram, const std::vector<float>& filterCenterFrequenciesHz) {
    HearingProfile profile;
    profile.formulaType = currentFormulaType;

    auto it = formulas.find(currentFormulaType);
    if (it == formulas.end()) {
        it = formulas.find(PrescriptionFormulaType::NAL_R);
    }

    if (it != formulas.end() && it->second) {
        profile.leftEarProfile = it->second->calculateEarProfile(audiogram.getEar(Ear::Left), filterCenterFrequenciesHz);
        profile.rightEarProfile = it->second->calculateEarProfile(audiogram.getEar(Ear::Right), filterCenterFrequenciesHz);
    }

    return profile;
}

std::vector<std::pair<PrescriptionFormulaType, std::string>> FittingEngine::getAvailableFormulas() const {
    return {
        {PrescriptionFormulaType::NAL_R, "NAL-R (National Acoustic Laboratories)"},
        {PrescriptionFormulaType::HalfGain, "Half-Gain Rule (1/2 Loss)"},
        {PrescriptionFormulaType::POGO, "POGO (Low-Cut Speech Target)"},
        {PrescriptionFormulaType::NAL_NL2, "NAL-NL2 (Non-Linear Multi-Level)"}
    };
}

} // namespace Amplify
