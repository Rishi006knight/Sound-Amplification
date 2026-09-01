#pragma once

#include "Core/HearingProfile.h"
#include <functional>

namespace Amplify {

class DSPControlView {
public:
    DSPControlView();
    ~DSPControlView() = default;

    void setMasterGain(float gainDb);
    float getMasterGain() const { return masterGain; }

    void setBassTrim(float trimDb);
    float getBassTrim() const { return bassTrim; }

    void setPresenceTrim(float trimDb);
    float getPresenceTrim() const { return presenceTrim; }

    void setTrebleTrim(float trimDb);
    float getTrebleTrim() const { return trebleTrim; }

    void setSafetyLimiterCeiling(float ceilingDbSPL);
    float getSafetyLimiterCeiling() const { return safetyCeiling; }

    void setOnControlsChanged(std::function<void()> cb) { onControlsChanged = cb; }

private:
    float masterGain{0.0f};
    float bassTrim{0.0f};
    float presenceTrim{0.0f};
    float trebleTrim{0.0f};
    float safetyCeiling{88.0f};

    std::function<void()> onControlsChanged{nullptr};
};

} // namespace Amplify
