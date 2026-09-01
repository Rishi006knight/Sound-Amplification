#include "FittingView.h"

namespace Amplify {

FittingView::FittingView() {
    setBounds(0, 0, 600, 300);
}

void FittingView::setHearingProfile(const HearingProfile& profile) {
    currentProfile = profile;
}

void FittingView::setBounds(int x, int y, int width, int height) {
    viewX = x;
    viewY = y;
    viewWidth = width;
    viewHeight = height;
}

void FittingView::setFormulaType(PrescriptionFormulaType type) {
    currentProfile.formulaType = type;
    if (onFormulaChanged) onFormulaChanged(type);
}

std::vector<FittingView::GainCurvePoint> FittingView::getLeftCurve() const {
    std::vector<GainCurvePoint> curve;
    for (const auto& bt : currentProfile.leftEarProfile.bandTargets) {
        curve.push_back({bt.centerFrequencyHz, bt.gainMediumDb});
    }
    return curve;
}

std::vector<FittingView::GainCurvePoint> FittingView::getRightCurve() const {
    std::vector<GainCurvePoint> curve;
    for (const auto& bt : currentProfile.rightEarProfile.bandTargets) {
        curve.push_back({bt.centerFrequencyHz, bt.gainMediumDb});
    }
    return curve;
}

} // namespace Amplify
