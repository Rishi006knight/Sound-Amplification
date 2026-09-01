#pragma once
#include <array>
#include <algorithm>

class GainCalculator
{
public:
    static constexpr int numBands = 6;

    // Converts audiogram thresholds (dB HL) to target filter gains (dB)
    // Using the Half-Gain Rule: Gain = Loss * 0.5
    static std::array<float, numBands> calculateGains(std::array<float, numBands> thresholdsDb, bool isSimulation)
    {
        std::array<float, numBands> gainsDb;
        for (int i = 0; i < numBands; ++i)
        {
            float rawGain = thresholdsDb[i] * 0.5f; // Half-gain rule
            
            // Clamp to a safe maximum of 30dB gain per band for prototype safety
            rawGain = std::clamp(rawGain, 0.0f, 30.0f);

            if (isSimulation)
                gainsDb[i] = -thresholdsDb[i]; // Invert for simulation
            else
                gainsDb[i] = rawGain;
        }
        return gainsDb;
    }
};
