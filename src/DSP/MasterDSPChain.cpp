#include "MasterDSPChain.h"
#include <algorithm>

namespace Amplify {

MasterDSPChain::MasterDSPChain() {
    prepare(48000.0f);
}

void MasterDSPChain::prepare(float sr) {
    sampleRate = sr;
    filterBank.prepare(sampleRate);
    lossSimulator.prepare(sampleRate);
    safetyLimiter.prepare(sampleRate, 1.5f, 50.0f);

    for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
        leftCompressors[b].prepare(sampleRate);
        rightCompressors[b].prepare(sampleRate);
    }

    for (size_t ch = 0; ch < 2; ++ch) {
        bassEQ[ch].setPeakingEQ(sampleRate, 250.0f, 0.0f, 1.0f);
        presenceEQ[ch].setPeakingEQ(sampleRate, 2500.0f, 0.0f, 1.0f);
        trebleEQ[ch].setPeakingEQ(sampleRate, 6000.0f, 0.0f, 1.0f);
    }

    reset();
}

void MasterDSPChain::reset() {
    filterBank.reset();
    lossSimulator.reset();
    safetyLimiter.reset();

    for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
        leftCompressors[b].reset();
        rightCompressors[b].reset();
    }

    for (size_t ch = 0; ch < 2; ++ch) {
        bassEQ[ch].reset();
        presenceEQ[ch].reset();
        trebleEQ[ch].reset();
    }

    inPeak = 0.0f;
    inRmsSum = 0.0f;
    outPeak = 0.0f;
    outRmsSum = 0.0f;
    meterSampleCount = 0;
}

void MasterDSPChain::setMode(ProcessingMode mode) {
    currentMode.store(mode);
}

void MasterDSPChain::updateProfile(const HearingProfile& profile, const AudiogramData& audiogram) {
    std::lock_guard<std::mutex> lock(profileMutex);

    lossSimulator.updateProfile(audiogram);
    safetyLimiter.setCeilingDbSPL(profile.safetyLimiterCeilingDbSPL);

    // Update WDRC compressors for each band
    const auto& freqs = FilterBank::getCenterFrequencies();

    for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
        float f = freqs[b];

        // Left ear
        float leftNomGain = profile.leftEarProfile.getNominalGain(f) + profile.masterGainDb;
        float leftCR = profile.leftEarProfile.getCompressionRatio(f);
        leftCompressors[b].setParameters(leftNomGain, leftCR, 50.0f, 8.0f, 80.0f, 6.0f);

        // Right ear
        float rightNomGain = profile.rightEarProfile.getNominalGain(f) + profile.masterGainDb;
        float rightCR = profile.rightEarProfile.getCompressionRatio(f);
        rightCompressors[b].setParameters(rightNomGain, rightCR, 50.0f, 8.0f, 80.0f, 6.0f);
    }

    // User tone trims
    for (size_t ch = 0; ch < 2; ++ch) {
        bassEQ[ch].setPeakingEQ(sampleRate, 250.0f, profile.bassTrimDb, 0.8f);
        presenceEQ[ch].setPeakingEQ(sampleRate, 2500.0f, profile.presenceTrimDb, 1.0f);
        trebleEQ[ch].setPeakingEQ(sampleRate, 6000.0f, profile.trebleTrimDb, 0.8f);
    }
}

void MasterDSPChain::processBlock(const float* inputLeft, const float* inputRight,
                                  float* outputLeft, float* outputRight, size_t numSamples) {
    ProcessingMode mode = currentMode.load();

    float localInPeak = 0.0f;
    float localInRms = 0.0f;
    float localOutPeak = 0.0f;
    float localOutRms = 0.0f;

    std::array<float, FilterBank::NUM_BANDS> leftBands;
    std::array<float, FilterBank::NUM_BANDS> rightBands;

    for (size_t i = 0; i < numSamples; ++i) {
        float inL = inputLeft ? inputLeft[i] : 0.0f;
        float inR = inputRight ? inputRight[i] : 0.0f;

        // Track Input Meters
        float absIn = std::max(std::abs(inL), std::abs(inR));
        if (absIn > localInPeak) localInPeak = absIn;
        localInRms += (inL * inL + inR * inR) * 0.5f;

        StereoFrame processedFrame{inL, inR};

        if (mode == ProcessingMode::Bypass) {
            // Mode A: Raw bypass (no hearing compensation)
            processedFrame = { inL, inR };
        } else if (mode == ProcessingMode::HearingLossSimulation) {
            // Mode B: Simulation of sensorineural hearing loss
            lossSimulator.setEnabled(true);
            processedFrame = lossSimulator.processFrame({ inL, inR });
        } else {
            // Mode C: Full Personalized Amplification (Filter Bank + WDRC + Limiter)
            filterBank.processSample(inL, 0, leftBands);
            filterBank.processSample(inR, 1, rightBands);

            float sumL = 0.0f;
            float sumR = 0.0f;

            for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
                float compL = leftCompressors[b].processSample(leftBands[b]);
                float compR = rightCompressors[b].processSample(rightBands[b]);
                sumL += compL;
                sumR += compR;
            }

            // Apply fine-tuning user EQ
            sumL = bassEQ[0].processSample(sumL);
            sumL = presenceEQ[0].processSample(sumL);
            sumL = trebleEQ[0].processSample(sumL);

            sumR = bassEQ[1].processSample(sumR);
            sumR = presenceEQ[1].processSample(sumR);
            sumR = trebleEQ[1].processSample(sumR);

            // MPO Brickwall Limiter to guarantee hearing safety
            processedFrame = safetyLimiter.processFrame({ sumL, sumR });
        }

        outputLeft[i] = processedFrame.left;
        outputRight[i] = processedFrame.right;

        // Track Output Meters
        float absOut = std::max(std::abs(processedFrame.left), std::abs(processedFrame.right));
        if (absOut > localOutPeak) localOutPeak = absOut;
        localOutRms += (processedFrame.left * processedFrame.left + processedFrame.right * processedFrame.right) * 0.5f;
    }

    // Update atomic meters
    inPeak = std::max<float>(inPeak.load() * 0.95f, localInPeak);
    inRmsSum = localInRms / std::max<size_t>(1, numSamples);
    outPeak = std::max<float>(outPeak.load() * 0.95f, localOutPeak);
    outRmsSum = localOutRms / std::max<size_t>(1, numSamples);
}

void MasterDSPChain::processInterleaved(const float* inputInterleaved, float* outputInterleaved, size_t numFrames) {
    std::vector<float> inL(numFrames);
    std::vector<float> inR(numFrames);
    std::vector<float> outL(numFrames);
    std::vector<float> outR(numFrames);

    for (size_t i = 0; i < numFrames; ++i) {
        inL[i] = inputInterleaved[i * 2];
        inR[i] = inputInterleaved[i * 2 + 1];
    }

    processBlock(inL.data(), inR.data(), outL.data(), outR.data(), numFrames);

    for (size_t i = 0; i < numFrames; ++i) {
        outputInterleaved[i * 2] = outL[i];
        outputInterleaved[i * 2 + 1] = outR[i];
    }
}

MeterData MasterDSPChain::getMeterData() {
    MeterData md;
    md.inputPeakDb = AudioBufferUtils::gainToDb(inPeak.load());
    md.inputRmsDb = AudioBufferUtils::gainToDb(std::sqrt(std::max(1e-9f, inRmsSum.load())));
    md.outputPeakDb = AudioBufferUtils::gainToDb(outPeak.load());
    md.outputRmsDb = AudioBufferUtils::gainToDb(std::sqrt(std::max(1e-9f, outRmsSum.load())));
    md.limiterGainReductionDb = safetyLimiter.getCurrentLimitingReductionDb();

    for (size_t b = 0; b < FilterBank::NUM_BANDS; ++b) {
        md.leftBandReductionDb[b] = leftCompressors[b].getCurrentGainReductionDb();
        md.rightBandReductionDb[b] = rightCompressors[b].getCurrentGainReductionDb();
    }

    return md;
}

} // namespace Amplify
