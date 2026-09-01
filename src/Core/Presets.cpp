#include "Presets.h"

namespace Amplify {

AudiogramData Presets::createNormalHearing() {
    AudiogramData data;
    data.patientName = "Normal Hearing (Baseline)";
    data.notes = "All thresholds within normal limits (-10 to 15 dB HL).";
    for (float f : AudiogramData::getStandardFrequencies()) {
        data.setThreshold(Ear::Left, f, 5.0f);
        data.setThreshold(Ear::Right, f, 5.0f);
    }
    return data;
}

AudiogramData Presets::createMildHighFrequencyLoss() {
    AudiogramData data;
    data.patientName = "Mild High-Frequency Loss";
    data.notes = "Typical early-stage presbycusis or subtle high-frequency loss.";
    
    // Frequencies: 125, 250, 500, 750, 1k, 1.5k, 2k, 3k, 4k, 6k, 8k
    std::vector<std::pair<float, float>> losses = {
        {125.0f, 10.0f}, {250.0f, 10.0f}, {500.0f, 15.0f}, {750.0f, 15.0f},
        {1000.0f, 20.0f}, {1500.0f, 25.0f}, {2000.0f, 35.0f}, {3000.0f, 40.0f},
        {4000.0f, 45.0f}, {6000.0f, 50.0f}, {8000.0f, 55.0f}
    };

    for (const auto& [f, db] : losses) {
        data.setThreshold(Ear::Left, f, db);
        data.setThreshold(Ear::Right, f, db + 2.0f);
    }
    return data;
}

AudiogramData Presets::createModeratePresbycusis() {
    AudiogramData data;
    data.patientName = "Moderate Sloping Presbycusis";
    data.notes = "Classic age-related sensorineural hearing loss with steep high-frequency roll-off.";
    
    std::vector<std::pair<float, float>> losses = {
        {125.0f, 15.0f}, {250.0f, 20.0f}, {500.0f, 25.0f}, {750.0f, 30.0f},
        {1000.0f, 35.0f}, {1500.0f, 45.0f}, {2000.0f, 55.0f}, {3000.0f, 65.0f},
        {4000.0f, 70.0f}, {6000.0f, 75.0f}, {8000.0f, 80.0f}
    };

    for (const auto& [f, db] : losses) {
        data.setThreshold(Ear::Left, f, db);
        data.setThreshold(Ear::Right, f, db);
    }
    return data;
}

AudiogramData Presets::createNoiseInducedNotch() {
    AudiogramData data;
    data.patientName = "Noise-Induced Hearing Loss (4 kHz Notch)";
    data.notes = "Acoustic trauma pattern characterized by a prominent notch at 3000-4000 Hz with recovery at 8000 Hz.";
    
    std::vector<std::pair<float, float>> losses = {
        {125.0f, 10.0f}, {250.0f, 10.0f}, {500.0f, 15.0f}, {750.0f, 15.0f},
        {1000.0f, 20.0f}, {1500.0f, 25.0f}, {2000.0f, 35.0f}, {3000.0f, 55.0f},
        {4000.0f, 65.0f}, {6000.0f, 50.0f}, {8000.0f, 30.0f}
    };

    for (const auto& [f, db] : losses) {
        data.setThreshold(Ear::Left, f, db);
        data.setThreshold(Ear::Right, f, db + 5.0f);
    }
    return data;
}

AudiogramData Presets::createModerateFlatLoss() {
    AudiogramData data;
    data.patientName = "Moderate Flat Sensorineural Loss";
    data.notes = "Evenly distributed ~45 dB loss across all speech frequencies.";
    
    for (float f : AudiogramData::getStandardFrequencies()) {
        data.setThreshold(Ear::Left, f, 45.0f);
        data.setThreshold(Ear::Right, f, 45.0f);
    }
    return data;
}

AudiogramData Presets::createCookieBiteLoss() {
    AudiogramData data;
    data.patientName = "Mid-Frequency (Cookie-Bite) Loss";
    data.notes = "Genetic / congenital pattern where mid-frequencies (500 Hz - 2 kHz) are affected while low and high frequencies remain preserved.";
    
    std::vector<std::pair<float, float>> losses = {
        {125.0f, 15.0f}, {250.0f, 20.0f}, {500.0f, 45.0f}, {750.0f, 55.0f},
        {1000.0f, 60.0f}, {1500.0f, 55.0f}, {2000.0f, 45.0f}, {3000.0f, 30.0f},
        {4000.0f, 25.0f}, {6000.0f, 20.0f}, {8000.0f, 15.0f}
    };

    for (const auto& [f, db] : losses) {
        data.setThreshold(Ear::Left, f, db);
        data.setThreshold(Ear::Right, f, db);
    }
    return data;
}

AudiogramData Presets::createAsymmetricLoss() {
    AudiogramData data;
    data.patientName = "Asymmetric Hearing Profile";
    data.notes = "Right ear normal, Left ear moderate high-frequency loss.";
    
    for (float f : AudiogramData::getStandardFrequencies()) {
        data.setThreshold(Ear::Right, f, 10.0f);
    }

    std::vector<std::pair<float, float>> leftLosses = {
        {125.0f, 15.0f}, {250.0f, 20.0f}, {500.0f, 25.0f}, {750.0f, 35.0f},
        {1000.0f, 45.0f}, {1500.0f, 50.0f}, {2000.0f, 60.0f}, {3000.0f, 65.0f},
        {4000.0f, 70.0f}, {6000.0f, 70.0f}, {8000.0f, 75.0f}
    };

    for (const auto& [f, db] : leftLosses) {
        data.setThreshold(Ear::Left, f, db);
    }
    return data;
}

std::vector<AudiogramPreset> Presets::getAllPresets() {
    return {
        {"normal", "Normal Hearing (Baseline)", "Standard baseline with no hearing impairment", createNormalHearing()},
        {"mild_presbycusis", "Mild High-Frequency Loss", "Subtle high-frequency age-related loss", createMildHighFrequencyLoss()},
        {"moderate_presbycusis", "Moderate Sloping Presbycusis", "Classic high-frequency loss with steep slope", createModeratePresbycusis()},
        {"noise_notch", "Noise-Induced Notch (4 kHz)", "Acoustic trauma with typical 4 kHz dip", createNoiseInducedNotch()},
        {"cookie_bite", "Cookie-Bite (Mid-Frequency Loss)", "Genetic mid-frequency speech dip", createCookieBiteLoss()},
        {"flat_moderate", "Moderate Flat Loss", "Uniform 45 dB loss across all frequencies", createModerateFlatLoss()},
        {"asymmetric", "Asymmetric Hearing Loss", "Right ear normal, Left ear moderate loss", createAsymmetricLoss()}
    };
}

AudiogramPreset Presets::getPresetById(const std::string& id) {
    auto all = getAllPresets();
    for (const auto& p : all) {
        if (p.id == id) return p;
    }
    return all.front();
}

} // namespace Amplify
