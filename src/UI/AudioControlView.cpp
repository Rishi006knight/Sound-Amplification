#include "AudioControlView.h"

namespace Amplify {

AudioControlView::AudioControlView() {}

void AudioControlView::setSourceType(AudioSourceType type) {
    currentSourceType = type;
    if (onSourceTypeChanged) onSourceTypeChanged(type);
}

void AudioControlView::setInputDevices(const std::vector<AudioDeviceInfo>& devices) {
    inputDevices = devices;
}

void AudioControlView::setOutputDevices(const std::vector<AudioDeviceInfo>& devices) {
    outputDevices = devices;
}

void AudioControlView::setSelectedInputDevice(const std::string& id) {
    selectedInputId = id;
    if (onInputDeviceChanged) onInputDeviceChanged(id);
}

void AudioControlView::setSelectedOutputDevice(const std::string& id) {
    selectedOutputId = id;
    if (onOutputDeviceChanged) onOutputDeviceChanged(id);
}

void AudioControlView::setEstimatedLatencyMs(float latencyMs) {
    estimatedLatencyMs = latencyMs;
}

} // namespace Amplify
