#include "PersonalizedDSP.h"

PersonalizedDSP::PersonalizedDSP() {}
PersonalizedDSP::~PersonalizedDSP() {}

void PersonalizedDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;

    // Prepare filters
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

    // Prepare compressor
    safetyCompressor.prepare(spec);
    safetyCompressor.reset();
    
    // Basic compressor settings to act as a safety limiter for now.
    // In later phases, these values will be mapped to the user's UCL.
    safetyCompressor.setThreshold(-10.0f); // -10 dBFS
    safetyCompressor.setRatio(10.0f);      // 10:1 ratio (heavy limiting)
    safetyCompressor.setAttack(2.0f);      // 2ms fast attack
    safetyCompressor.setRelease(50.0f);    // 50ms release

    // Initialize coefficients to flat (0 dB gain)
    updateFilterCoefficients();
}

void PersonalizedDSP::reset()
{
    for (auto& filter : leftFilters)  filter.reset();
    for (auto& filter : rightFilters) filter.reset();
    safetyCompressor.reset();
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
    // Recalculate Left Ear Filters
    for (size_t i = 0; i < numBands; ++i)
    {
        // Convert dB gain to linear gain factor
        float linearGain = juce::Decibels::decibelsToGain(leftGainsDb[i]);
        
        *leftFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, frequencies[i], qFactor, linearGain);
    }

    // Recalculate Right Ear Filters
    for (size_t i = 0; i < numBands; ++i)
    {
        float linearGain = juce::Decibels::decibelsToGain(rightGainsDb[i]);
        
        *rightFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, frequencies[i], qFactor, linearGain);
    }
}

void PersonalizedDSP::process(juce::AudioBuffer<float>& buffer)
{
    // If for some reason we have no audio channels, bail out
    if (buffer.getNumChannels() == 0) return;

    juce::dsp::AudioBlock<float> block(buffer);
    
    // 1. Process Stereo Audio through Filters independently
    // We extract single channel blocks from the main stereo block
    if (buffer.getNumChannels() >= 2)
    {
        auto leftBlock = block.getSingleChannelBlock(0);
        auto rightBlock = block.getSingleChannelBlock(1);

        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

        // Run signal through Left EQ bank
        for (auto& filter : leftFilters)
            filter.process(leftContext);

        // Run signal through Right EQ bank
        for (auto& filter : rightFilters)
            filter.process(rightContext);
    }
    else if (buffer.getNumChannels() == 1)
    {
        // Fallback for mono audio (apply left ear profile)
        auto monoBlock = block.getSingleChannelBlock(0);
        juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);
        for (auto& filter : leftFilters)
            filter.process(monoContext);
    }

    // 2. Process through Safety Compressor (applies to whole stereo block)
    juce::dsp::ProcessContextReplacing<float> compressorContext(block);
    safetyCompressor.process(compressorContext);
}
