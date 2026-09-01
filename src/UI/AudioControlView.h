#pragma once

#include "Audio/AudioTypes.h"
#include <vector>
#include <string>
#include <functional>

namespace Amplify {

class AudioControlView {
public:
    AudioControlView();
    ~AudioControlView() = default;

    void setSourceType(AudioSourceType type);
    AudioSourceType getSourceType() const { return currentSourceType; }

    void setInputDevices(const std::vector<AudioDeviceInfo>& devices);
    void setOutputDevices(const std::vector<AudioDeviceInfo>& devices);

    void setSelectedInputDevice(const std::string& id);
    void setSelectedOutputDevice(const std::string& id);

    void setEstimatedLatencyMs(float latencyMs);
    float getEstimatedLatencyMs() const { return estimatedLatencyMs; }

    void setOnSourceTypeChanged(std::function<void(AudioSourceType)> cb) { onSourceTypeChanged = cb; }
    void setOnInputDeviceChanged(std::function<void(const std::string&)> cb) { onInputDeviceChanged = cb; }
    void setOnOutputDeviceChanged(std::function<void(const std::string&)> cb) { onOutputDeviceChanged = cb; }
    void setOnPlayTonePulse(std::function<void(float freq, float dbSPL, float durMs, bool left, bool right)> cb) {
        onPlayTonePulse = cb;
    }

    void triggerTone(float freqHz, float dbSPL, float durMs = 1000.0f, bool left = true, bool right = true) {
        if (onPlayTonePulse) onPlayTonePulse(freqHz, dbSPL, durMs, left, right);
    }

private:
    AudioSourceType currentSourceType{AudioSourceType::WASAPISystemLoopback};
    std::vector<AudioDeviceInfo> inputDevices;
    std::vector<AudioDeviceInfo> outputDevices;

    std::string selectedInputId{""};
    std::string selectedOutputId{""};
    float estimatedLatencyMs{12.0f};

    std::function<void(AudioSourceType)> onSourceTypeChanged{nullptr};
    std::function<void(const std::string&)> onInputDeviceChanged{nullptr};
    std::function<void(const std::string&)> onOutputDeviceChanged{nullptr};
    std::function<void(float, float, float, bool, bool)> onPlayTonePulse{nullptr};
};

} // namespace Amplify
