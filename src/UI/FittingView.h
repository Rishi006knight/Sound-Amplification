#pragma once

#include "Core/HearingProfile.h"
#include "Fitting/FittingEngine.h"
#include <vector>
#include <string>
#include <functional>

namespace Amplify {

class FittingView {
public:
    FittingView();
    ~FittingView() = default;

    void setHearingProfile(const HearingProfile& profile);
    const HearingProfile& getHearingProfile() const { return currentProfile; }

    void setBounds(int x, int y, int width, int height);

    void setFormulaType(PrescriptionFormulaType type);
    PrescriptionFormulaType getFormulaType() const { return currentProfile.formulaType; }

    void setOnFormulaChanged(std::function<void(PrescriptionFormulaType)> cb) { onFormulaChanged = cb; }

    struct GainCurvePoint {
        float frequencyHz;
        float gainDb;
    };

    std::vector<GainCurvePoint> getLeftCurve() const;
    std::vector<GainCurvePoint> getRightCurve() const;

private:
    HearingProfile currentProfile;
    int viewX{0}, viewY{0}, viewWidth{600}, viewHeight{300};
    std::function<void(PrescriptionFormulaType)> onFormulaChanged{nullptr};
};

} // namespace Amplify
