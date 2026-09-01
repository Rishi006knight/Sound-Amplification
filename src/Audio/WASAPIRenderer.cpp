#include "WASAPIRenderer.h"
#include "WASAPILoopback.h"
#include <iostream>
#include <chrono>

#ifdef _WIN32
#pragma comment(lib, "Avrt.lib")
#include <avrt.h>
#endif

namespace Amplify {

WASAPIRenderer::WASAPIRenderer() {
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif
}

WASAPIRenderer::~WASAPIRenderer() {
    stopRender();
#ifdef _WIN32
    CoUninitialize();
#endif
}

std::vector<AudioDeviceInfo> WASAPIRenderer::enumerateRenderEndpoints() {
    return WASAPILoopback::enumerateRenderDevices();
}

bool WASAPIRenderer::startRender(const std::string& deviceId, AudioRenderCallback callback) {
    if (rendering.load()) {
        stopRender();
    }

    renderCallback = callback;
    rendering = true;

#ifdef _WIN32
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(hr)) {
        rendering = false;
        return false;
    }

    if (deviceId.empty()) {
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &targetDevice);
    } else {
        int wSize = MultiByteToWideChar(CP_UTF8, 0, deviceId.c_str(), -1, nullptr, 0);
        std::wstring wId(wSize, 0);
        MultiByteToWideChar(CP_UTF8, 0, deviceId.c_str(), -1, &wId[0], wSize);
        hr = deviceEnumerator->GetDevice(wId.c_str(), &targetDevice);
    }

    if (FAILED(hr) || !targetDevice) {
        if (deviceEnumerator) { deviceEnumerator->Release(); deviceEnumerator = nullptr; }
        rendering = false;
        return false;
    }

    hr = targetDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);
    if (FAILED(hr) || !audioClient) {
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        rendering = false;
        return false;
    }

    WAVEFORMATEX* pwfx = nullptr;
    hr = audioClient->GetMixFormat(&pwfx);
    if (FAILED(hr) || !pwfx) {
        audioClient->Release(); audioClient = nullptr;
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        rendering = false;
        return false;
    }

    sampleRate = pwfx->nSamplesPerSec;
    channels = pwfx->nChannels;

    REFERENCE_TIME hnsBufferDuration = 500000; // 50 ms buffer
    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                 hnsBufferDuration, 0, pwfx, nullptr);
    CoTaskMemFree(pwfx);

    if (FAILED(hr)) {
        audioClient->Release(); audioClient = nullptr;
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        rendering = false;
        return false;
    }

    hRenderEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    audioClient->SetEventHandle(hRenderEvent);

    hr = audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&renderClient);
    if (FAILED(hr) || !renderClient) {
        CloseHandle(hRenderEvent);
        audioClient->Release(); audioClient = nullptr;
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        rendering = false;
        return false;
    }

    audioClient->Start();
#endif

    renderThread = std::thread(&WASAPIRenderer::renderLoop, this);
    return true;
}

void WASAPIRenderer::stopRender() {
    if (!rendering.load()) return;

    rendering = false;
#ifdef _WIN32
    if (hRenderEvent) {
        SetEvent(hRenderEvent);
    }
#endif

    if (renderThread.joinable()) {
        renderThread.join();
    }

#ifdef _WIN32
    if (audioClient) {
        audioClient->Stop();
    }
    if (renderClient) {
        renderClient->Release();
        renderClient = nullptr;
    }
    if (audioClient) {
        audioClient->Release();
        audioClient = nullptr;
    }
    if (hRenderEvent) {
        CloseHandle(hRenderEvent);
        hRenderEvent = NULL;
    }
    if (targetDevice) {
        targetDevice->Release();
        targetDevice = nullptr;
    }
    if (deviceEnumerator) {
        deviceEnumerator->Release();
        deviceEnumerator = nullptr;
    }
#endif
}

void WASAPIRenderer::renderLoop() {
#ifdef _WIN32
    DWORD taskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristicsA("Pro Audio", &taskIndex);

    UINT32 bufferFrameCount = 0;
    audioClient->GetBufferSize(&bufferFrameCount);

    std::vector<float> stagingBuffer(bufferFrameCount * channels, 0.0f);

    while (rendering.load()) {
        DWORD waitResult = WaitForSingleObject(hRenderEvent, 50);
        if (!rendering.load()) break;

        if (waitResult == WAIT_OBJECT_0) {
            UINT32 numPaddingFrames = 0;
            HRESULT hr = audioClient->GetCurrentPadding(&numPaddingFrames);
            if (SUCCEEDED(hr)) {
                UINT32 numFramesNeeded = bufferFrameCount - numPaddingFrames;
                if (numFramesNeeded > 0) {
                    BYTE* pData = nullptr;
                    hr = renderClient->GetBuffer(numFramesNeeded, &pData);
                    if (SUCCEEDED(hr) && pData) {
                        float* pFloatDst = reinterpret_cast<float*>(pData);

                        if (renderCallback) {
                            renderCallback(pFloatDst, numFramesNeeded, sampleRate, channels);
                        } else {
                            std::fill(pFloatDst, pFloatDst + numFramesNeeded * channels, 0.0f);
                        }

                        renderClient->ReleaseBuffer(numFramesNeeded, 0);
                    }
                }
            }
        }
    }

    if (hTask) {
        AvRevertMmThreadCharacteristics(hTask);
    }
#else
    while (rendering.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
#endif
}

} // namespace Amplify
