#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "PersonalizedDSP.h"
#include "Audiometry/ThresholdData.h"
#include "HearingProfile/FittingAlgorithms.h"

class HearingAssistProcessor : public juce::AudioProcessor
{
public:
    HearingAssistProcessor();
    ~HearingAssistProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    void releaseResources() override {}

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HearingAssist"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int /*index*/) override {}
    const juce::String getProgramName(int /*index*/) override { return {}; }
    void changeProgramName(int /*index*/, const juce::String& /*newName*/) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Direct access to APVTS and DSP
    juce::AudioProcessorValueTreeState apvts;
    HearingAssist::PersonalizedDSP& getDSP() { return dspEngine; }

    void loadPreset(const HearingAssist::ThresholdData& preset);

private:
    HearingAssist::PersonalizedDSP dspEngine;
    HearingAssist::ThresholdData currentAudiogram;

    // Cache of active parameters
    std::array<float, 6> lastLeftGains { -999.0f, -999.0f, -999.0f, -999.0f, -999.0f, -999.0f };
    std::array<float, 6> lastRightGains { -999.0f, -999.0f, -999.0f, -999.0f, -999.0f, -999.0f };
    int lastAuditionMode{-1};
    int lastFormula{-1};
    float lastMasterGain{-999.0f};
    float lastBassTrim{-999.0f};
    float lastTrebleTrim{-999.0f};

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HearingAssistProcessor)
};
