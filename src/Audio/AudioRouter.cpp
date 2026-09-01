#include "AudioRouter.h"
#include <algorithm>

namespace Amplify {

LockFreeRingBuffer::LockFreeRingBuffer(size_t capacityFrames, size_t channels)
    : capacity(capacityFrames), numChannels(channels), buffer(capacityFrames * channels, 0.0f) {}

void LockFreeRingBuffer::reset() {
    writeHead.store(0);
    readHead.store(0);
    std::fill(buffer.begin(), buffer.end(), 0.0f);
}

size_t LockFreeRingBuffer::getAvailableRead() const {
    size_t w = writeHead.load(std::memory_order_acquire);
    size_t r = readHead.load(std::memory_order_relaxed);
    if (w >= r) return w - r;
    return capacity - (r - w);
}

size_t LockFreeRingBuffer::getAvailableWrite() const {
    return capacity - 1 - getAvailableRead();
}

size_t LockFreeRingBuffer::write(const float* srcInterleaved, size_t numFrames) {
    size_t avail = getAvailableWrite();
    size_t toWrite = std::min(numFrames, avail);
    if (toWrite == 0) return 0;

    size_t w = writeHead.load(std::memory_order_relaxed);
    for (size_t i = 0; i < toWrite; ++i) {
        size_t idx = ((w + i) % capacity) * numChannels;
        for (size_t ch = 0; ch < numChannels; ++ch) {
            buffer[idx + ch] = srcInterleaved[i * numChannels + ch];
        }
    }

    writeHead.store((w + toWrite) % capacity, std::memory_order_release);
    return toWrite;
}

size_t LockFreeRingBuffer::read(float* dstInterleaved, size_t numFrames) {
    size_t avail = getAvailableRead();
    size_t toRead = std::min(numFrames, avail);

    size_t r = readHead.load(std::memory_order_relaxed);
    for (size_t i = 0; i < toRead; ++i) {
        size_t idx = ((r + i) % capacity) * numChannels;
        for (size_t ch = 0; ch < numChannels; ++ch) {
            dstInterleaved[i * numChannels + ch] = buffer[idx + ch];
        }
    }

    // Fill remaining with silence if underrun
    if (toRead < numFrames) {
        std::fill(dstInterleaved + toRead * numChannels, dstInterleaved + numFrames * numChannels, 0.0f);
    }

    readHead.store((r + toRead) % capacity, std::memory_order_release);
    return toRead;
}

AudioRouter::AudioRouter() {}

AudioRouter::~AudioRouter() {
    stop();
}

bool AudioRouter::initialize(MasterDSPChain* dspChain) {
    masterDSP = dspChain;
    wavePlayer.prepare(48000.0f);
    ringBuffer.reset();
    return true;
}

void AudioRouter::shutdown() {
    stop();
    masterDSP = nullptr;
}

bool AudioRouter::start() {
    ringBuffer.reset();

    // Start WASAPI output renderer
    bool renderOk = renderer.startRender(currentOutputDeviceId, [this](float* data, size_t frames, unsigned int sr, unsigned int ch) {
        handleRenderRequest(data, frames, sr, ch);
    });

    if (sourceType == AudioSourceType::WASAPISystemLoopback) {
        loopback.startCapture(currentInputDeviceId, [this](const float* data, size_t frames, unsigned int sr, unsigned int ch) {
            handleCaptureData(data, frames, sr, ch);
        });
    }

    return renderOk;
}

void AudioRouter::stop() {
    loopback.stopCapture();
    renderer.stopRender();
    ringBuffer.reset();
}

void AudioRouter::setSourceType(AudioSourceType type) {
    sourceType = type;
    if (sourceType == AudioSourceType::WASAPISystemLoopback) {
        if (!loopback.isCapturing()) {
            loopback.startCapture(currentInputDeviceId, [this](const float* data, size_t frames, unsigned int sr, unsigned int ch) {
                handleCaptureData(data, frames, sr, ch);
            });
        }
    } else {
        loopback.stopCapture();
    }
}

void AudioRouter::setInputDevice(const std::string& deviceId) {
    currentInputDeviceId = deviceId;
    if (loopback.isCapturing()) {
        loopback.stopCapture();
        loopback.startCapture(currentInputDeviceId, [this](const float* data, size_t frames, unsigned int sr, unsigned int ch) {
            handleCaptureData(data, frames, sr, ch);
        });
    }
}

void AudioRouter::setOutputDevice(const std::string& deviceId) {
    currentOutputDeviceId = deviceId;
    if (renderer.isRendering()) {
        renderer.stopRender();
        renderer.startRender(currentOutputDeviceId, [this](float* data, size_t frames, unsigned int sr, unsigned int ch) {
            handleRenderRequest(data, frames, sr, ch);
        });
    }
}

void AudioRouter::handleCaptureData(const float* interleavedData, size_t numFrames, unsigned int /*sampleRate*/, unsigned int /*channels*/) {
    if (sourceType == AudioSourceType::WASAPISystemLoopback) {
        ringBuffer.write(interleavedData, numFrames);
    }
}

void AudioRouter::handleRenderRequest(float* interleavedData, size_t numFrames, unsigned int sampleRate, unsigned int channels) {
    std::vector<float> inputBuffer(numFrames * channels, 0.0f);

    if (sourceType == AudioSourceType::WASAPISystemLoopback) {
        ringBuffer.read(inputBuffer.data(), numFrames);
    } else {
        wavePlayer.fillBuffer(inputBuffer.data(), numFrames, channels);
    }

    if (masterDSP) {
        masterDSP->processInterleaved(inputBuffer.data(), interleavedData, numFrames);
    } else {
        std::copy(inputBuffer.begin(), inputBuffer.end(), interleavedData);
    }
}

float AudioRouter::getEstimatedLatencyMs() const {
    size_t queuedFrames = ringBuffer.getAvailableRead();
    return (static_cast<float>(queuedFrames) / 48000.0f) * 1000.0f + 10.0f; // plus ~10ms hardware buffer
}

} // namespace Amplify
