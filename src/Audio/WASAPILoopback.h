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

using AudioCaptureCallback = std::function<void(const float* interleavedData, size_t numFrames, unsigned int sampleRate, unsigned int channels)>;

class WASAPILoopback {
public:
    WASAPILoopback();
    ~WASAPILoopback();

    static std::vector<AudioDeviceInfo> enumerateRenderDevices();

    bool startCapture(const std::string& deviceId = "", AudioCaptureCallback callback = nullptr);
    void stopCapture();

    bool isCapturing() const { return capturing.load(); }
    unsigned int getSampleRate() const { return sampleRate; }
    unsigned int getChannels() const { return channels; }

private:
    std::atomic<bool> capturing{false};
    AudioCaptureCallback dataCallback{nullptr};
    std::thread captureThread;

    unsigned int sampleRate{48000};
    unsigned int channels{2};

    void captureLoop();

#ifdef _WIN32
    IMMDeviceEnumerator* deviceEnumerator{nullptr};
    IMMDevice* targetDevice{nullptr};
    IAudioClient* audioClient{nullptr};
    IAudioCaptureClient* captureClient{nullptr};
    HANDLE hCaptureEvent{NULL};
#endif
};

} // namespace Amplify
