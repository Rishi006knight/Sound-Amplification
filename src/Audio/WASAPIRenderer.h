#pragma once

#include "AudioTypes.h"
#include <functional>
#include <thread>
#include <atomic>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#endif

namespace Amplify {

using AudioRenderCallback = std::function<void(float* interleavedData, size_t numFrames, unsigned int sampleRate, unsigned int channels)>;

class WASAPIRenderer {
public:
    WASAPIRenderer();
    ~WASAPIRenderer();

    static std::vector<AudioDeviceInfo> enumerateRenderEndpoints();

    bool startRender(const std::string& deviceId = "", AudioRenderCallback callback = nullptr);
    void stopRender();

    bool isRendering() const { return rendering.load(); }
    unsigned int getSampleRate() const { return sampleRate; }
    unsigned int getChannels() const { return channels; }

private:
    std::atomic<bool> rendering{false};
    AudioRenderCallback renderCallback{nullptr};
    std::thread renderThread;

    unsigned int sampleRate{48000};
    unsigned int channels{2};

    void renderLoop();

#ifdef _WIN32
    IMMDeviceEnumerator* deviceEnumerator{nullptr};
    IMMDevice* targetDevice{nullptr};
    IAudioClient* audioClient{nullptr};
    IAudioRenderClient* renderClient{nullptr};
    HANDLE hRenderEvent{NULL};
#endif
};

} // namespace Amplify
