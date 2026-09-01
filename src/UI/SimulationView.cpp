#include "SimulationView.h"
#include <algorithm>

namespace Amplify {

SimulationView::SimulationView() {}

void SimulationView::setMode(ProcessingMode mode) {
    activeMode = mode;
    if (onModeChanged) onModeChanged(mode);
}

void SimulationView::setRecruitmentIntensity(float factor) {
    recruitmentFactor = std::clamp(factor, 0.0f, 1.0f);
}

} // namespace Amplify
