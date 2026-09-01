#include "SpectrumVisualizer.h"

namespace Amplify {

SpectrumVisualizer::SpectrumVisualizer() {}

void SpectrumVisualizer::updateMeters(const MeterData& data) {
    cachedMeters = data;
}

} // namespace Amplify
