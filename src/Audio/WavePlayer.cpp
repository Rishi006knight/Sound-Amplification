#include "WavePlayer.h"
#include <cmath>
#include <numbers>

namespace Amplify {

WavePlayer::WavePlayer() : rng(1337) {
    prepare(48000.0f);
}

void WavePlayer::prepare(float sr) {
    sampleRate = sr;
    tonePhase = 0.0f;
    speechFormantPhase1 = 0.0f;
    speechFormantPhase2 = 0.0f;
    speechFundPhase = 0.0f;
    remainingPulseSamples = 0;
}

void WavePlayer::setGeneratorType(GeneratorType type) {
    currentType.store(type);
}

void WavePlayer::setPureTone(float frequencyHz, float levelDbSPL, bool playLeft, bool playRight) {
    toneFreq = frequencyHz;
    toneLevelLinear = AudioBufferUtils::dbSPLToFloat(levelDbSPL, 100.0f);
    toneLeftEar = playLeft;
    toneRightEar = playRight;
    currentType.store(GeneratorType::PureTone);
}

void WavePlayer::triggerTonePulse(float frequencyHz, float levelDbSPL, float durationMs, bool playLeft, bool playRight) {
    toneFreq = frequencyHz;
    toneLevelLinear = AudioBufferUtils::dbSPLToFloat(levelDbSPL, 100.0f);
    toneLeftEar = playLeft;
    toneRightEar = playRight;
    remainingPulseSamples.store(static_cast<int>(sampleRate * (durationMs * 0.001f)));
    currentType.store(GeneratorType::PureTone);
}

void WavePlayer::fillBuffer(float* interleavedDst, size_t numFrames, unsigned int channels) {
    GeneratorType type = currentType.load();

    if (type == GeneratorType::Silence) {
        std::fill(interleavedDst, interleavedDst + numFrames * channels, 0.0f);
        return;
    }

    const float twoPi = 2.0f * 3.14159265358979323846f;
    float phaseInc = twoPi * toneFreq / sampleRate;

    for (size_t i = 0; i < numFrames; ++i) {
        float sampleL = 0.0f;
        float sampleR = 0.0f;

        if (type == GeneratorType::PureTone) {
            int pulse = remainingPulseSamples.load();
            if (pulse > 0) {
                remainingPulseSamples.fetch_sub(1);
            } else if (pulse == 0 && remainingPulseSamples != -1) {
                // Pulse ended
                sampleL = 0.0f;
                sampleR = 0.0f;
            }

            float tone = std::sin(tonePhase) * toneLevelLinear;
            tonePhase += phaseInc;
            if (tonePhase >= twoPi) tonePhase -= twoPi;

            if (toneLeftEar) sampleL = tone;
            if (toneRightEar) sampleR = tone;
        } else if (type == GeneratorType::PinkNoise) {
            // Paul Kellet's filtered pink noise generator
            float white = whiteDist(rng) * 0.2f;
            b0 = 0.99886f * b0 + white * 0.0555179f;
            b1 = 0.99332f * b1 + white * 0.0750759f;
            b2 = 0.96900f * b2 + white * 0.1538520f;
            b3 = 0.86650f * b3 + white * 0.3104856f;
            b4 = 0.55000f * b4 + white * 0.5329522f;
            b5 = -0.7616f * b5 - white * 0.0168980f;
            float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
            b6 = white * 0.115926f;

            sampleL = pink * 0.15f;
            sampleR = pink * 0.15f;
        } else if (type == GeneratorType::SpeechSampleDemo) {
            // Harmonic speech spectrum simulation (F0=130Hz, F1=700Hz, F2=1800Hz, F3=2800Hz)
            float f0Inc = twoPi * 130.0f / sampleRate;
            float f1Inc = twoPi * 700.0f / sampleRate;
            float f2Inc = twoPi * 1800.0f / sampleRate;
            float f3Inc = twoPi * 2800.0f / sampleRate;

            speechFundPhase += f0Inc;
            speechFormantPhase1 += f1Inc;
            speechFormantPhase2 += f2Inc;
            if (speechFundPhase >= twoPi) speechFundPhase -= twoPi;
            if (speechFormantPhase1 >= twoPi) speechFormantPhase1 -= twoPi;
            if (speechFormantPhase2 >= twoPi) speechFormantPhase2 -= twoPi;

            float pulse = (std::sin(speechFundPhase) > 0.0f) ? 1.0f : 0.0f;
            float s = 0.3f * std::sin(speechFundPhase) +
                      0.2f * std::sin(speechFormantPhase1) * pulse +
                      0.15f * std::sin(speechFormantPhase2) * pulse +
                      0.05f * whiteDist(rng);

            sampleL = s * 0.25f;
            sampleR = s * 0.25f;
        }

        if (channels >= 2) {
            interleavedDst[i * channels] = sampleL;
            interleavedDst[i * channels + 1] = sampleR;
        } else if (channels == 1) {
            interleavedDst[i] = (sampleL + sampleR) * 0.5f;
        }
    }
}

} // namespace Amplify
