#include "DSPControlView.h"
#include <algorithm>

namespace Amplify {

DSPControlView::DSPControlView() {}

void DSPControlView::setMasterGain(float gainDb) {
    masterGain = std::clamp(gainDb, -18.0f, 18.0f);
    if (onControlsChanged) onControlsChanged();
}

void DSPControlView::setBassTrim(float trimDb) {
    bassTrim = std::clamp(trimDb, -12.0f, 12.0f);
    if (onControlsChanged) onControlsChanged();
}

void DSPControlView::setPresenceTrim(float trimDb) {
    presenceTrim = std::clamp(trimDb, -12.0f, 12.0f);
    if (onControlsChanged) onControlsChanged();
}

void DSPControlView::setTrebleTrim(float trimDb) {
    trebleTrim = std::clamp(trimDb, -12.0f, 12.0f);
    if (onControlsChanged) onControlsChanged();
}

void DSPControlView::setSafetyLimiterCeiling(float ceilingDbSPL) {
    safetyCeiling = std::clamp(ceilingDbSPL, 75.0f, 95.0f);
    if (onControlsChanged) onControlsChanged();
}

} // namespace Amplify
