#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "AudiogramComponent.h"

class HearingAssistProcessor;

class AudiogramView : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    AudiogramView(HearingAssistProcessor& p);
    ~AudiogramView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    HearingAssistProcessor& processor;

    // Audiogram Interactive Graph
    HearingAssist::AudiogramComponent audiogramGraph;

    // 12 Sliders for Left (Blue) and Right (Red) ears
    juce::Slider leftSliders[6];
    juce::Slider rightSliders[6];
    juce::Label freqLabels[6];

    // Presets Combo Box
    juce::ComboBox presetBox;

    // 3-Way Audition Mode Buttons
    juce::TextButton bypassButton{ "Bypass (Clean)" };
    juce::TextButton simButton{ "Loss Simulation" };
    juce::TextButton ampButton{ "Personalized Amplification" };

    // Fitting Formula Selector
    juce::ComboBox formulaBox;

    // Fine-Tuning Controls
    juce::Slider masterGainSlider;
    juce::Slider bassTrimSlider;
    juce::Slider trebleTrimSlider;
    juce::Slider limiterCeilingSlider;

    juce::Label masterGainLabel{ {}, "Master Gain" };
    juce::Label bassTrimLabel{ {}, "Bass Trim" };
    juce::Label trebleTrimLabel{ {}, "Treble Trim" };
    juce::Label limiterCeilingLabel{ {}, "Safety MPO" };

    // Status / Metering Labels
    juce::Label vuMeterLabel;
    juce::Label lossDegreeLabel;

    void updateSliderParam(int index, bool isLeft);
    void updateAudiogramGraphFromSliders();
    void selectPreset(int presetId);
    void updateModeButtons(int activeMode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudiogramView)
};
