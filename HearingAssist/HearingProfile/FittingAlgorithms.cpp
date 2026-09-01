#include "FittingAlgorithms.h"
#include <cmath>
#include <algorithm>

namespace HearingAssist {

float FittingAlgorithms::getNALRKFactor(float freqHz) {
    if (freqHz <= 250.0f) return -17.0f;
    if (freqHz <= 500.0f) return -8.0f;
    if (freqHz <= 1000.0f) return +1.0f;
    if (freqHz <= 2000.0f) return -1.0f;
    if (freqHz <= 4000.0f) return -2.0f;
    return -4.0f; // 8000 Hz
}

float FittingAlgorithms::calculateNALRInsertionGain(float freqHz, float htl, float pta3) {
    float k = getNALRKFactor(freqHz);
    float X = 0.05f * pta3;
    float gain = X + 0.31f * htl + k;
    return std::clamp(gain, 0.0f, 32.0f);
}

FittingTargets FittingAlgorithms::calculateTargets(const ThresholdData& audiogram,
                                                   PrescriptionFormula formula,
                                                   bool isSimulation,
                                                   float masterGainDb,
                                                   float bassTrimDb,
                                                   float trebleTrimDb) {
    FittingTargets targets;
    const auto& freqs = ThresholdData::getStandardFrequencies();
    float leftPTA = audiogram.calculatePTA3(Ear::Left);
    float rightPTA = audiogram.calculatePTA3(Ear::Right);

    for (size_t i = 0; i < 6; ++i) {
        float f = freqs[i];
        float leftLoss = audiogram.getAirThreshold(Ear::Left, f);
        float rightLoss = audiogram.getAirThreshold(Ear::Right, f);

        float leftGain = 0.0f;
        float rightGain = 0.0f;

        if (isSimulation) {
            // Simulation Mode: Invert threshold to simulate attenuation & muffled perception
            leftGain = -std::clamp(leftLoss, 0.0f, 70.0f);
            rightGain = -std::clamp(rightLoss, 0.0f, 70.0f);
            targets.compressionRatios[i] = 1.0f;
        } else {
            switch (formula) {
                case PrescriptionFormula::HalfGain:
                    leftGain = leftLoss * 0.5f;
                    rightGain = rightLoss * 0.5f;
                    break;

                case PrescriptionFormula::POGO:
                    leftGain = leftLoss * 0.5f;
                    rightGain = rightLoss * 0.5f;
                    if (f <= 250.0f) {
                        leftGain -= 10.0f;
                        rightGain -= 10.0f;
                    } else if (f <= 500.0f) {
                        leftGain -= 5.0f;
                        rightGain -= 5.0f;
                    }
                    break;

                case PrescriptionFormula::NAL_R:
                    leftGain = calculateNALRInsertionGain(f, leftLoss, leftPTA);
                    rightGain = calculateNALRInsertionGain(f, rightLoss, rightPTA);
                    break;

                case PrescriptionFormula::NAL_NL2: {
                    float baseL = calculateNALRInsertionGain(f, leftLoss, leftPTA);
                    float baseR = calculateNALRInsertionGain(f, rightLoss, rightPTA);
                    // Non-linear moderate target
                    leftGain = baseL * 1.05f;
                    rightGain = baseR * 1.05f;
                    break;
                }
            }

            // Clamp max gain safely
            leftGain = std::clamp(leftGain, 0.0f, 32.0f);
            rightGain = std::clamp(rightGain, 0.0f, 32.0f);

            // User fine-tuning trims
            float trim = masterGainDb;
            if (f <= 500.0f) trim += bassTrimDb;
            if (f >= 4000.0f) trim += trebleTrimDb;

            leftGain = std::clamp(leftGain + trim, 0.0f, 35.0f);
            rightGain = std::clamp(rightGain + trim, 0.0f, 35.0f);

            // Calculate compression ratio from dynamic range
            float leftDR = audiogram.getDynamicRange(Ear::Left, f);
            float rightDR = audiogram.getDynamicRange(Ear::Right, f);
            targets.compressionRatios[i] = std::clamp(65.0f / std::min(leftDR, rightDR), 1.0f, 3.5f);
        }

        targets.leftGainsDb[i] = leftGain;
        targets.rightGainsDb[i] = rightGain;
    }

    return targets;
}

std::string FittingAlgorithms::getFormulaName(PrescriptionFormula formula) {
    switch (formula) {
        case PrescriptionFormula::HalfGain: return "Half-Gain Rule (1/2 Loss)";
        case PrescriptionFormula::POGO: return "POGO (Low-Cut Speech Target)";
        case PrescriptionFormula::NAL_R: return "NAL-R (Speech Intelligibility)";
        case PrescriptionFormula::NAL_NL2: return "NAL-NL2 (Non-Linear Multi-Level)";
        default: return "Custom";
    }
}

std::vector<std::pair<PrescriptionFormula, std::string>> FittingAlgorithms::getAvailableFormulas() {
    return {
        { PrescriptionFormula::NAL_R, "NAL-R (Speech Intelligibility)" },
        { PrescriptionFormula::NAL_NL2, "NAL-NL2 (Non-Linear Multi-Level)" },
        { PrescriptionFormula::HalfGain, "Half-Gain Rule (1/2 Loss)" },
        { PrescriptionFormula::POGO, "POGO (Low-Cut Speech Target)" }
    };
}

} // namespace HearingAssist
