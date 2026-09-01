#pragma once

#include <string>
#include "Audiometry/ThresholdData.h"
#include "HearingProfile/FittingAlgorithms.h"

namespace HearingAssist {

class ProfileStore {
public:
    static std::string exportAudiogramToJson(const ThresholdData& data);
    static bool importAudiogramFromJson(const std::string& json, ThresholdData& outData);

    static bool saveToFile(const std::string& filepath, const ThresholdData& data);
    static bool loadFromFile(const std::string& filepath, ThresholdData& outData);
};

} // namespace HearingAssist
