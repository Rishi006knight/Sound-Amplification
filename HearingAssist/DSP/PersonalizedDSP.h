#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

/**
 * The core DSP engine for personalized hearing amplification.
 * Uses a 6-band IIR peaking filter bank for Left and Right channels independently,
 * followed by a dynamic range compressor for safety and UCL (Uncomfortable Loud Level) management.
 */
class PersonalizedDSP
{
public:
    PersonalizedDSP();
    ~PersonalizedDSP();

    /** Prepares the DSP pipeline with processing specifications. */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state of the filters and compressor. */
    void reset();

    /**
     * Updates the filter gains based on the user's hearing profile.
     * @param leftGainsDb  Array of 6 gain values in dB for Left ear (250, 500, 1k, 2k, 4k, 8k Hz)
     * @param rightGainsDb Array of 6 gain values in dB for Right ear (250, 500, 1k, 2k, 4k, 8k Hz)
     */
    void updateProfile(const std::array<float, 6>& leftGainsDb, 
                       const std::array<float, 6>& rightGainsDb);

    /** Processes an audio buffer in place. */
    void process(juce::AudioBuffer<float>& buffer);

private:
    /** Internal helper to recalculate IIR filter coefficients. */
    void updateFilterCoefficients();

    double currentSampleRate { 48000.0 };

    // Standard audiometric frequencies
    static constexpr int numBands = 6;
    const std::array<float, numBands> frequencies = { 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f };
    const float qFactor = 1.5f; // Q factor for peaking filters (1.0-2.0 is typical for hearing aid bands)

    // Current gain targets in dB
    std::array<float, numBands> leftGainsDb { 0.0f };
    std::array<float, numBands> rightGainsDb { 0.0f };

    // IIR Filters
    std::array<juce::dsp::IIR::Filter<float>, numBands> leftFilters;
    std::array<juce::dsp::IIR::Filter<float>, numBands> rightFilters;

    // Safety Compressor to prevent output from exceeding UCL
    juce::dsp::Compressor<float> safetyCompressor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PersonalizedDSP)
};
