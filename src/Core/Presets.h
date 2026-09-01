#pragma once

#include "AudiogramData.h"
#include <string>
#include <vector>

namespace Amplify {

struct AudiogramPreset {
    std::string id;
    std::string name;
    std::string description;
    AudiogramData data;
};

class Presets {
public:
    static std::vector<AudiogramPreset> getAllPresets();
    static AudiogramPreset getPresetById(const std::string& id);

    static AudiogramData createNormalHearing();
    static AudiogramData createMildHighFrequencyLoss();
    static AudiogramData createModeratePresbycusis();
    static AudiogramData createNoiseInducedNotch();
    static AudiogramData createModerateFlatLoss();
    static AudiogramData createCookieBiteLoss();
    static AudiogramData createAsymmetricLoss();
};

} // namespace Amplify
