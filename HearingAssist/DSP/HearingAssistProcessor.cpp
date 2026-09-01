#include "HearingAssistProcessor.h"
#include "UI/AudiogramView.h"

HearingAssistProcessor::HearingAssistProcessor()
    : juce::AudioProcessor(BusesProperties()
                           .withInput("Input", juce::AudioChannelSet::stereo(), true)
                           .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameters())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout HearingAssistProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // 6 Sliders for Left Ear (0-100 dB HL)
    for (int i = 0; i < 6; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("L_GAIN" + std::to_string(i), 1),
            "Left " + std::to_string(i),
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            0.0f));
    }

    // 6 Sliders for Right Ear (0-100 dB HL)
    for (int i = 0; i < 6; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("R_GAIN" + std::to_string(i), 1),
            "Right " + std::to_string(i),
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            0.0f));
    }

    // Simulation Mode Toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("SIM_MODE", 1),
        "Simulation Mode",
        false));

    return { params.begin(), params.end() };
}

void HearingAssistProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    dspEngine.prepare(spec);
}

void HearingAssistProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear extra channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. Read parameters from UI safely via atomic pointers
    bool simMode = apvts.getRawParameterValue("SIM_MODE")->load() > 0.5f;
    std::array<float, 6> leftThresholds{};
    std::array<float, 6> rightThresholds{};

    for (int i = 0; i < 6; ++i)
    {
        leftThresholds[i] = apvts.getRawParameterValue("L_GAIN" + std::to_string(i))->load();
        rightThresholds[i] = apvts.getRawParameterValue("R_GAIN" + std::to_string(i))->load();
    }

    // 2. Calculate Gains (Half-gain rule or Simulation inversion)
    std::array<float, 6> targetLeftGains = GainCalculator::calculateGains(leftThresholds, simMode);
    std::array<float, 6> targetRightGains = GainCalculator::calculateGains(rightThresholds, simMode);

    // 3. Check if coefficients need updating (only if values changed)
    bool needsUpdate = (simMode != lastSimState);

    for (int i = 0; i < 6; ++i)
    {
        if (std::abs(targetLeftGains[i] - lastLeftGains[i]) > 0.1f) needsUpdate = true;
        if (std::abs(targetRightGains[i] - lastRightGains[i]) > 0.1f) needsUpdate = true;
    }

    if (needsUpdate)
    {
        dspEngine.updateProfile(targetLeftGains, targetRightGains);
        lastLeftGains = targetLeftGains;
        lastRightGains = targetRightGains;
        lastSimState = simMode;
    }

    // 4. Process audio through DSP engine
    dspEngine.process(buffer);
}

void HearingAssistProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HearingAssistProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* HearingAssistProcessor::createEditor()
{
    return new AudiogramView(*this);
}

// JUCE AudioProcessor creation factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HearingAssistProcessor();
}
