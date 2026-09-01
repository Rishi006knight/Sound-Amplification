#pragma once

#include "Core/AudiogramData.h"
#include "Core/HearingProfile.h"
#include <vector>
#include <string>

namespace Amplify {

class IFittingFormula {
public:
    virtual ~IFittingFormula() = default;

    virtual PrescriptionFormulaType getType() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;

    // Calculates target insertion gain (dB) and WDRC band targets for an ear
    virtual EarProfile calculateEarProfile(const EarAudiogram& audiogram, const std::vector<float>& targetFrequenciesHz) = 0;
};

} // namespace Amplify
