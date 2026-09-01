#pragma once

#include "AudioTypes.h"
#include "WASAPILoopback.h"
#include "WASAPIRenderer.h"
#include "WavePlayer.h"
#include "DSP/MasterDSPChain.h"
#include <memory>
#include <mutex>
#include <vector>

namespace Amplify {

class LockFreeRingBuffer {
public:
    explicit LockFreeRingBuffer(size_t capacityFrames = 16384, size_t channels = 2);

    void reset();
    size_t write(const float* srcInterleaved, size_t numFrames);
    size_t read(float* dstInterleaved, size_t numFrames);
    size_t getAvailableRead() const;
    size_t getAvailableWrite() const;

private:
    size_t capacity;
    size_t numChannels;
    std::vector<float> buffer;
    std::atomic<size_t> writeHead{0};
    std::atomic<size_t> readHead{0};
};

class AudioRouter {
public:
    AudioRouter();
    ~AudioRouter();

    bool initialize(MasterDSPChain* dspChain);
    void shutdown();

    bool start();
    void stop();

    void setSourceType(AudioSourceType type);
    AudioSourceType getSourceType() const { return sourceType; }

    void setInputDevice(const std::string& deviceId);
    void setOutputDevice(const std::string& deviceId);

    WavePlayer& getWavePlayer() { return wavePlayer; }
    WASAPILoopback& getLoopback() { return loopback; }
    WASAPIRenderer& getRenderer() { return renderer; }

    float getEstimatedLatencyMs() const;

private:
    MasterDSPChain* masterDSP{nullptr};
    AudioSourceType sourceType{AudioSourceType::WASAPISystemLoopback};

    std::string currentInputDeviceId{""};
    std::string currentOutputDeviceId{""};

    WASAPILoopback loopback;
    WASAPIRenderer renderer;
    WavePlayer wavePlayer;

    LockFreeRingBuffer ringBuffer{32768, 2};

    void handleCaptureData(const float* interleavedData, size_t numFrames, unsigned int sampleRate, unsigned int channels);
    void handleRenderRequest(float* interleavedData, size_t numFrames, unsigned int sampleRate, unsigned int channels);
};

} // namespace Amplify
