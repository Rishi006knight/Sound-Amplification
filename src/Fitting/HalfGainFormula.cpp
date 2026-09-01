#include "HalfGainFormula.h"
#include <cmath>
#include <algorithm>

namespace Amplify {

HalfGainFormula::HalfGainFormula(bool usePOGOCorrections)
    : usePOGO(usePOGOCorrections) {}

EarProfile HalfGainFormula::calculateEarProfile(const EarAudiogram& audiogram, const std::vector<float>& targetFrequenciesHz) {
    EarProfile profile;
    profile.ear = audiogram.ear;

    for (float f : targetFrequenciesHz) {
        float htl = audiogram.getAirThreshold(f);
        float ucl = audiogram.getUCL(f);
        float dynRange = audiogram.getDynamicRange(f);

        float gain = 0.5f * htl;

        if (usePOGO) {
            if (f <= 250.0f) {
                gain -= 10.0f;
            } else if (f <= 500.0f) {
                gain -= 5.0f;
            }
        }

        gain = std::max(0.0f, gain);

        BandTarget bt;
        bt.centerFrequencyHz = f;
        bt.gainMediumDb = gain;
        bt.gainSoftDb = gain + std::clamp(htl * 0.10f, 0.0f, 5.0f);
        bt.gainLoudDb = std::max(0.0f, gain - std::clamp(htl * 0.12f, 0.0f, 6.0f));
        bt.compressionRatio = std::clamp(65.0f / dynRange, 1.0f, 3.0f);
        bt.kneepointDbSPL = 52.0f;
        bt.mpoDbSPL = std::clamp(ucl, 80.0f, 105.0f);

        profile.bandTargets.push_back(bt);
    }

    return profile;
}

} // namespace Amplify
