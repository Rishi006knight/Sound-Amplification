#pragma once

#include <string>
#include <vector>

namespace Amplify {

struct AudioDeviceInfo {
    std::string id;
    std::string name;
    bool isDefault{false};
    bool isInput{false};
    unsigned int sampleRate{48000};
    unsigned int channels{2};
};

enum class AudioSourceType {
    WASAPISystemLoopback,
    InternalToneGenerator,
    AudioFilePlayback
};

} // namespace Amplify
