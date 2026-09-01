#pragma once

#include "Core/AudiogramData.h"
#include "Core/HearingProfile.h"
#include <string>

namespace Amplify {

class ProfileSerializer {
public:
    static std::string serializeAudiogramToJson(const AudiogramData& audiogram);
    static bool deserializeAudiogramFromJson(const std::string& jsonString, AudiogramData& outAudiogram);

    static std::string serializeHearingProfileToJson(const HearingProfile& profile);
    static bool deserializeHearingProfileFromJson(const std::string& jsonString, HearingProfile& outProfile);

    static bool saveAudiogramToFile(const std::string& filePath, const AudiogramData& audiogram);
    static bool loadAudiogramFromFile(const std::string& filePath, AudiogramData& outAudiogram);

    static bool saveProfileToFile(const std::string& filePath, const HearingProfile& profile);
    static bool loadProfileFromFile(const std::string& filePath, HearingProfile& outProfile);
};

} // namespace Amplify
