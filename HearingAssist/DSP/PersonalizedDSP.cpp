#include "PersonalizedDSP.h"
#include <cmath>

namespace HearingAssist {

PersonalizedDSP::PersonalizedDSP() {}
PersonalizedDSP::~PersonalizedDSP() {}

void PersonalizedDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;

    for (auto& filter : leftFilters)
    {
        filter.prepare(spec);
        filter.reset();
    }
    for (auto& filter : rightFilters)
    {
        filter.prepare(spec);
        filter.reset();
    }

    safetyCompressor.prepare(spec);
    safetyCompressor.reset();
    safetyCompressor.setThreshold(-12.0f);
    safetyCompressor.setRatio(6.0f);
    safetyCompressor.setAttack(3.0f);
    safetyCompressor.setRelease(60.0f);

    outputLimiter.prepare(spec);
    outputLimiter.reset();
    outputLimiter.setThreshold(-1.0f);
    outputLimiter.setRelease(40.0f);

    updateFilterCoefficients();
}

void PersonalizedDSP::reset()
{
    for (auto& filter : leftFilters)  filter.reset();
    for (auto& filter : rightFilters) filter.reset();
    safetyCompressor.reset();
    outputLimiter.reset();
}

void PersonalizedDSP::setLimiterCeiling(float ceilingDbFS)
{
    outputLimiter.setThreshold(std::clamp(ceilingDbFS, -20.0f, -0.5f));
}

void PersonalizedDSP::updateProfile(const std::array<float, 6>& newLeftGainsDb,
                                   const std::array<float, 6>& newRightGainsDb)
{
    leftGainsDb = newLeftGainsDb;
    rightGainsDb = newRightGainsDb;
    updateFilterCoefficients();
}

void PersonalizedDSP::updateFilterCoefficients()
{
    for (size_t i = 0; i < numBands; ++i)
    {
        float gainLinearL = juce::Decibels::decibelsToGain(leftGainsDb[i]);
        *leftFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, frequencies[i], qFactor, gainLinearL);

        float gainLinearR = juce::Decibels::decibelsToGain(rightGainsDb[i]);
        *rightFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, frequencies[i], qFactor, gainLinearR);
    }
}

void PersonalizedDSP::process(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0) return;

    // Track input metering
    float rawInPeak = buffer.getMagnitude(0, buffer.getNumSamples());
    float rawInRms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    inPeak.store(juce::Decibels::gainToDecibels(rawInPeak, -100.0f));
    inRms.store(juce::Decibels::gainToDecibels(rawInRms, -100.0f));

    ProcessingMode mode = currentMode.load();

    if (mode != ProcessingMode::Bypass)
    {
        juce::dsp::AudioBlock<float> block(buffer);

        if (buffer.getNumChannels() >= 2)
        {
            auto leftBlock = block.getSingleChannelBlock(0);
            auto rightBlock = block.getSingleChannelBlock(1);

            juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
            juce::dsp::ProcessContextReplacing<float> rightContext(rightContext);

            for (auto& filter : leftFilters)
                filter.process(leftContext);

            for (auto& filter : rightFilters)
                filter.process(rightContext);
        }
        else if (buffer.getNumChannels() == 1)
        {
            auto monoBlock = block.getSingleChannelBlock(0);
            juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);
            for (auto& filter : leftFilters)
                filter.process(monoContext);
        }

        // Apply compressor and brickwall safety limiter
        juce::dsp::ProcessContextReplacing<float> blockContext(block);
        safetyCompressor.process(blockContext);
        outputLimiter.process(blockContext);
    }

    // Track output metering
    float rawOutPeak = buffer.getMagnitude(0, buffer.getNumSamples());
    float rawOutRms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    outPeak.store(juce::Decibels::gainToDecibels(rawOutPeak, -100.0f));
    outRms.store(juce::Decibels::gainToDecibels(rawOutRms, -100.0f));
}

MeteringInfo PersonalizedDSP::getMeteringInfo() const
{
    MeteringInfo info;
    info.inputPeakDb = inPeak.load();
    info.inputRmsDb = inRms.load();
    info.outputPeakDb = outPeak.load();
    info.outputRmsDb = outRms.load();
    info.isLimitingActive = limiterActive.load();
    return info;
}

} // namespace HearingAssist
