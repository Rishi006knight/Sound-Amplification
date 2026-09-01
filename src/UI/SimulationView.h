#pragma once

#include "DSP/MasterDSPChain.h"
#include "Core/Presets.h"
#include <functional>
#include <string>

namespace Amplify {

class SimulationView {
public:
    SimulationView();
    ~SimulationView() = default;

    void setMode(ProcessingMode mode);
    ProcessingMode getMode() const { return activeMode; }

    void setRecruitmentIntensity(float factor);
    float getRecruitmentIntensity() const { return recruitmentFactor; }

    void setOnModeChanged(std::function<void(ProcessingMode)> cb) { onModeChanged = cb; }
    void setOnPresetSelected(std::function<void(const std::string&)> cb) { onPresetSelected = cb; }

private:
    ProcessingMode activeMode{ProcessingMode::PersonalizedAmplification};
    float recruitmentFactor{0.7f};

    std::function<void(ProcessingMode)> onModeChanged{nullptr};
    std::function<void(const std::string&)> onPresetSelected{nullptr};
};

} // namespace Amplify
