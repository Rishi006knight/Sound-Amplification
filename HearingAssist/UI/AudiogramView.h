#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class HearingAssistProcessor;

class AudiogramView : public juce::AudioProcessorEditor
{
public:
    AudiogramView(HearingAssistProcessor& p);
    ~AudiogramView() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    HearingAssistProcessor& processor;

    juce::Slider leftSliders[6];
    juce::Slider rightSliders[6];
    juce::Label freqLabels[6];
    juce::ToggleButton simButton;

    void updateParam(int index, bool isLeft);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudiogramView)
};
