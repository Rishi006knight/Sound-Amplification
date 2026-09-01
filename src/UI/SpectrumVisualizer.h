#pragma once

#include "DSP/MasterDSPChain.h"
#include <vector>

namespace Amplify {

class SpectrumVisualizer {
public:
    SpectrumVisualizer();
    ~SpectrumVisualizer() = default;

    void updateMeters(const MeterData& data);

    float getInputPeakDb() const { return cachedMeters.inputPeakDb; }
    float getInputRmsDb() const { return cachedMeters.inputRmsDb; }
    float getOutputPeakDb() const { return cachedMeters.outputPeakDb; }
    float getOutputRmsDb() const { return cachedMeters.outputRmsDb; }
    float getLimiterReductionDb() const { return cachedMeters.limiterGainReductionDb; }

    const std::array<float, FilterBank::NUM_BANDS>& getLeftBandReductions() const {
        return cachedMeters.leftBandReductionDb;
    }

private:
    MeterData cachedMeters;
};

} // namespace Amplify
