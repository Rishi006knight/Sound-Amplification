#include "DashboardView.h"
#include "Core/Presets.h"

namespace Amplify {

DashboardView::DashboardView(FittingEngine& fe, MasterDSPChain& dsp, AudioRouter& ar)
    : fittingEngine(fe), dspChain(dsp), audioRouter(ar) {}

void DashboardView::initialize() {
    // 1. Initialize default audiogram (e.g. Mild High-Frequency Loss preset)
    currentAudiogram = Presets::createMildHighFrequencyLoss();
    audiogramView.setAudiogramData(currentAudiogram);

    // 2. Wire up callbacks
    audiogramView.setOnAudiogramChanged([this](const AudiogramData& data) {
        currentAudiogram = data;
        recalculateProfile();
    });

    fittingView.setOnFormulaChanged([this](PrescriptionFormulaType type) {
        fittingEngine.setFormula(type);
        recalculateProfile();
    });

    dspControlView.setOnControlsChanged([this]() {
        currentProfile.masterGainDb = dspControlView.getMasterGain();
        currentProfile.bassTrimDb = dspControlView.getBassTrim();
        currentProfile.presenceTrimDb = dspControlView.getPresenceTrim();
        currentProfile.trebleTrimDb = dspControlView.getTrebleTrim();
        currentProfile.safetyLimiterCeilingDbSPL = dspControlView.getSafetyLimiterCeiling();
        dspChain.updateProfile(currentProfile, currentAudiogram);
    });

    simulationView.setOnModeChanged([this](ProcessingMode mode) {
        dspChain.setMode(mode);
    });

    simulationView.setOnPresetSelected([this](const std::string& presetId) {
        loadPreset(presetId);
    });

    audioControlView.setOnSourceTypeChanged([this](AudioSourceType type) {
        audioRouter.setSourceType(type);
    });

    audioControlView.setOnPlayTonePulse([this](float freq, float dbSPL, float durMs, bool left, bool right) {
        audioRouter.getWavePlayer().triggerTonePulse(freq, dbSPL, durMs, left, right);
    });

    recalculateProfile();
}

void DashboardView::loadPreset(const std::string& presetId) {
    auto p = Presets::getPresetById(presetId);
    currentAudiogram = p.data;
    audiogramView.setAudiogramData(currentAudiogram);
    recalculateProfile();
}

void DashboardView::recalculateProfile() {
    const auto& freqs = FilterBank::getCenterFrequencies();
    currentProfile = fittingEngine.calculateProfile(currentAudiogram, freqs);

    currentProfile.masterGainDb = dspControlView.getMasterGain();
    currentProfile.bassTrimDb = dspControlView.getBassTrim();
    currentProfile.presenceTrimDb = dspControlView.getPresenceTrim();
    currentProfile.trebleTrimDb = dspControlView.getTrebleTrim();
    currentProfile.safetyLimiterCeilingDbSPL = dspControlView.getSafetyLimiterCeiling();

    fittingView.setHearingProfile(currentProfile);
    dspChain.updateProfile(currentProfile, currentAudiogram);
}

} // namespace Amplify
