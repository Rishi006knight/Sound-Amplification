#include "WASAPILoopback.h"
#include <iostream>
#include <chrono>

#ifdef _WIN32
#include <functiondiscoverykeys_devpkey.h>
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Mmdevapi.lib")
#endif

namespace Amplify {

WASAPILoopback::WASAPILoopback() {
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif
}

WASAPILoopback::~WASAPILoopback() {
    stopCapture();
#ifdef _WIN32
    CoUninitialize();
#endif
}

std::vector<AudioDeviceInfo> WASAPILoopback::enumerateRenderDevices() {
    std::vector<AudioDeviceInfo> devices;

#ifdef _WIN32
    IMMDeviceEnumerator* enumerator = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (SUCCEEDED(hr) && enumerator) {
        IMMDeviceCollection* collection = nullptr;
        hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
        if (SUCCEEDED(hr) && collection) {
            UINT count = 0;
            collection->GetCount(&count);

            IMMDevice* defaultDevice = nullptr;
            enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
            LPWSTR defaultId = nullptr;
            if (defaultDevice) {
                defaultDevice->GetId(&defaultId);
            }

            for (UINT i = 0; i < count; ++i) {
                IMMDevice* dev = nullptr;
                if (SUCCEEDED(collection->Item(i, &dev)) && dev) {
                    LPWSTR pId = nullptr;
                    dev->GetId(&pId);

                    IPropertyStore* props = nullptr;
                    std::string friendlyName = "Audio Endpoint " + std::to_string(i);

                    if (SUCCEEDED(dev->OpenPropertyStore(STGM_READ, &props)) && props) {
                        PROPVARIANT varName;
                        PropVariantInit(&varName);
                        if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &varName))) {
                            if (varName.pwszVal) {
                                int size = WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, nullptr, 0, nullptr, nullptr);
                                std::string str(size - 1, 0);
                                WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, &str[0], size, nullptr, nullptr);
                                friendlyName = str;
                            }
                        }
                        PropVariantClear(&varName);
                        props->Release();
                    }

                    AudioDeviceInfo info;
                    info.name = friendlyName;
                    if (pId) {
                        int size = WideCharToMultiByte(CP_UTF8, 0, pId, -1, nullptr, 0, nullptr, nullptr);
                        std::string idStr(size - 1, 0);
                        WideCharToMultiByte(CP_UTF8, 0, pId, -1, &idStr[0], size, nullptr, nullptr);
                        info.id = idStr;

                        if (defaultId && wcscmp(defaultId, pId) == 0) {
                            info.isDefault = true;
                        }
                        CoTaskMemFree(pId);
                    }

                    devices.push_back(info);
                    dev->Release();
                }
            }

            if (defaultId) CoTaskMemFree(defaultId);
            if (defaultDevice) defaultDevice->Release();
            collection->Release();
        }
        enumerator->Release();
    }
#else
    AudioDeviceInfo defaultDev;
    defaultDev.id = "default_render";
    defaultDev.name = "Default System Audio (Loopback)";
    defaultDev.isDefault = true;
    devices.push_back(defaultDev);
#endif

    return devices;
}

bool WASAPILoopback::startCapture(const std::string& deviceId, AudioCaptureCallback callback) {
    if (capturing.load()) {
        stopCapture();
    }

    dataCallback = callback;
    capturing = true;

#ifdef _WIN32
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(hr)) {
        capturing = false;
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
        capturing = false;
        return false;
    }

    hr = targetDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);
    if (FAILED(hr) || !audioClient) {
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        capturing = false;
        return false;
    }

    WAVEFORMATEX* pwfx = nullptr;
    hr = audioClient->GetMixFormat(&pwfx);
    if (FAILED(hr) || !pwfx) {
        audioClient->Release(); audioClient = nullptr;
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        capturing = false;
        return false;
    }

    sampleRate = pwfx->nSamplesPerSec;
    channels = pwfx->nChannels;

    // Initialize with LOOPBACK flag
    REFERENCE_TIME hnsBufferDuration = 1000000; // 100 ms buffer
    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                                 hnsBufferDuration, 0, pwfx, nullptr);
    CoTaskMemFree(pwfx);

    if (FAILED(hr)) {
        audioClient->Release(); audioClient = nullptr;
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        capturing = false;
        return false;
    }

    hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient);
    if (FAILED(hr) || !captureClient) {
        audioClient->Release(); audioClient = nullptr;
        targetDevice->Release(); targetDevice = nullptr;
        deviceEnumerator->Release(); deviceEnumerator = nullptr;
        capturing = false;
        return false;
    }

    audioClient->Start();
#endif

    captureThread = std::thread(&WASAPILoopback::captureLoop, this);
    return true;
}

void WASAPILoopback::stopCapture() {
    if (!capturing.load()) return;

    capturing = false;
    if (captureThread.joinable()) {
        captureThread.join();
    }

#ifdef _WIN32
    if (audioClient) {
        audioClient->Stop();
    }
    if (captureClient) {
        captureClient->Release();
        captureClient = nullptr;
    }
    if (audioClient) {
        audioClient->Release();
        audioClient = nullptr;
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

void WASAPILoopback::captureLoop() {
    std::vector<float> floatBuffer(4096 * channels);

    while (capturing.load()) {
#ifdef _WIN32
        UINT32 packetLength = 0;
        HRESULT hr = captureClient->GetNextPacketSize(&packetLength);

        if (SUCCEEDED(hr) && packetLength > 0) {
            BYTE* pData = nullptr;
            UINT32 numFramesAvailable = 0;
            DWORD flags = 0;

            hr = captureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            if (SUCCEEDED(hr) && pData) {
                if (floatBuffer.size() < numFramesAvailable * channels) {
                    floatBuffer.resize(numFramesAvailable * channels);
                }

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    std::fill(floatBuffer.begin(), floatBuffer.begin() + numFramesAvailable * channels, 0.0f);
                } else {
                    const float* pFloatSrc = reinterpret_cast<const float*>(pData);
                    for (size_t i = 0; i < numFramesAvailable * channels; ++i) {
                        floatBuffer[i] = pFloatSrc[i];
                    }
                }

                if (dataCallback) {
                    dataCallback(floatBuffer.data(), numFramesAvailable, sampleRate, channels);
                }

                captureClient->ReleaseBuffer(numFramesAvailable);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
#else
        // Simulation heartbeat
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
    }
}

} // namespace Amplify
