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

    // 6 Sliders for Left Ear (0 - 100 dB HL)
    for (int i = 0; i < 6; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("L_GAIN" + std::to_string(i), 1),
            "Left Ear " + std::to_string(i),
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            0.0f));
    }

    // 6 Sliders for Right Ear (0 - 100 dB HL)
    for (int i = 0; i < 6; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("R_GAIN" + std::to_string(i), 1),
            "Right Ear " + std::to_string(i),
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            0.0f));
    }

    // 3-Way Audition Mode: 0 = Bypass, 1 = Simulation, 2 = Amplification
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("AUDITION_MODE", 1),
        "Audition Mode",
        juce::StringArray{ "Bypass", "Simulation", "Personalized Amplification" },
        2));

    // Prescription Formula: 0 = NAL-R, 1 = NAL-NL2, 2 = HalfGain, 3 = POGO
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("FORMULA", 1),
        "Prescription Formula",
        juce::StringArray{ "NAL-R", "NAL-NL2", "HalfGain", "POGO" },
        0));

    // Fine-Tuning Trims & Safety Controls
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("MASTER_GAIN", 1),
        "Master Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.5f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("BASS_TRIM", 1),
        "Bass Trim",
        juce::NormalisableRange<float>(-10.0f, 10.0f, 0.5f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("TREBLE_TRIM", 1),
        "Treble Trim",
        juce::NormalisableRange<float>(-10.0f, 10.0f, 0.5f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("LIMITER_CEILING", 1),
        "Limiter Ceiling",
        juce::NormalisableRange<float>(-18.0f, -0.5f, 0.5f),
        -1.0f));

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

    // Clear unused output channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. Read APVTS parameters atomically
    int auditionModeIdx = static_cast<int>(apvts.getRawParameterValue("AUDITION_MODE")->load());
    int formulaIdx = static_cast<int>(apvts.getRawParameterValue("FORMULA")->load());

    float masterGain = apvts.getRawParameterValue("MASTER_GAIN")->load();
    float bassTrim = apvts.getRawParameterValue("BASS_TRIM")->load();
    float trebleTrim = apvts.getRawParameterValue("TREBLE_TRIM")->load();
    float limiterCeiling = apvts.getRawParameterValue("LIMITER_CEILING")->load();

    const auto& freqs = HearingAssist::ThresholdData::getStandardFrequencies();
    for (size_t i = 0; i < 6; ++i)
    {
        float lLoss = apvts.getRawParameterValue("L_GAIN" + std::to_string(i))->load();
        float rLoss = apvts.getRawParameterValue("R_GAIN" + std::to_string(i))->load();
        currentAudiogram.setAirThreshold(HearingAssist::Ear::Left, freqs[i], lLoss);
        currentAudiogram.setAirThreshold(HearingAssist::Ear::Right, freqs[i], rLoss);
    }

    // 2. Set DSP processing mode
    dspEngine.setMode(static_cast<HearingAssist::ProcessingMode>(auditionModeIdx));
    dspEngine.setLimiterCeiling(limiterCeiling);

    // 3. Compute fitting target gains using selected formula
    bool isSim = (auditionModeIdx == 1);
    auto prescriptionFormula = static_cast<HearingAssist::PrescriptionFormula>(formulaIdx);
    auto targets = HearingAssist::FittingAlgorithms::calculateTargets(
        currentAudiogram, prescriptionFormula, isSim, masterGain, bassTrim, trebleTrim);

    // 4. Update coefficients if parameters changed
    bool needsUpdate = (auditionModeIdx != lastAuditionMode || formulaIdx != lastFormula ||
                        std::abs(masterGain - lastMasterGain) > 0.05f ||
                        std::abs(bassTrim - lastBassTrim) > 0.05f ||
                        std::abs(trebleTrim - lastTrebleTrim) > 0.05f);

    for (int i = 0; i < 6; ++i)
    {
        if (std::abs(targets.leftGainsDb[i] - lastLeftGains[i]) > 0.1f) needsUpdate = true;
        if (std::abs(targets.rightGainsDb[i] - lastRightGains[i]) > 0.1f) needsUpdate = true;
    }

    if (needsUpdate)
    {
        dspEngine.updateProfile(targets.leftGainsDb, targets.rightGainsDb);
        lastLeftGains = targets.leftGainsDb;
        lastRightGains = targets.rightGainsDb;
        lastAuditionMode = auditionModeIdx;
        lastFormula = formulaIdx;
        lastMasterGain = masterGain;
        lastBassTrim = bassTrim;
        lastTrebleTrim = trebleTrim;
    }

    // 5. Run audio DSP pipeline
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

// Plugin filter factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HearingAssistProcessor();
}
