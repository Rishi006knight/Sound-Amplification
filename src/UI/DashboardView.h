#pragma once

#include "AudiogramView.h"
#include "FittingView.h"
#include "DSPControlView.h"
#include "SimulationView.h"
#include "AudioControlView.h"
#include "SpectrumVisualizer.h"
#include "Core/AudiogramData.h"
#include "Core/HearingProfile.h"
#include "Fitting/FittingEngine.h"
#include "DSP/MasterDSPChain.h"
#include "Audio/AudioRouter.h"

namespace Amplify {

enum class ActiveTab {
    AudiogramAssessment,
    HearingProfileFitting,
    DSPAndEnhancement,
    SimulationAndAudition,
    AudioRoutingSettings
};

class DashboardView {
public:
    DashboardView(FittingEngine& fittingEngine, MasterDSPChain& dspChain, AudioRouter& audioRouter);
    ~DashboardView() = default;

    void initialize();

    void setActiveTab(ActiveTab tab) { currentTab = tab; }
    ActiveTab getActiveTab() const { return currentTab; }

    void loadPreset(const std::string& presetId);
    void recalculateProfile();

    AudiogramView audiogramView;
    FittingView fittingView;
    DSPControlView dspControlView;
    SimulationView simulationView;
    AudioControlView audioControlView;
    SpectrumVisualizer spectrumVisualizer;

    const AudiogramData& getAudiogramData() const { return currentAudiogram; }
    const HearingProfile& getHearingProfile() const { return currentProfile; }

private:
    FittingEngine& fittingEngine;
    MasterDSPChain& dspChain;
    AudioRouter& audioRouter;

    ActiveTab currentTab{ActiveTab::AudiogramAssessment};

    AudiogramData currentAudiogram;
    HearingProfile currentProfile;
};

} // namespace Amplify
