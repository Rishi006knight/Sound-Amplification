#include "AudiogramView.h"
#include "DSP/HearingAssistProcessor.h"

AudiogramView::AudiogramView(HearingAssistProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    const juce::String freqs[6] = { "250", "500", "1k", "2k", "4k", "8k" };

    for (int i = 0; i < 6; ++i)
    {
        // Left ear sliders (Blue)
        leftSliders[i].setSliderStyle(juce::Slider::LinearVertical);
        leftSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 45, 16);
        leftSliders[i].setRange(0.0, 100.0, 1.0);
        leftSliders[i].setColour(juce::Slider::thumbColourId, juce::Colour(0xFF3182CE));
        leftSliders[i].setColour(juce::Slider::trackColourId, juce::Colour(0xFF2B6CB0));
        addAndMakeVisible(leftSliders[i]);

        // Right ear sliders (Red)
        rightSliders[i].setSliderStyle(juce::Slider::LinearVertical);
        rightSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 45, 16);
        rightSliders[i].setRange(0.0, 100.0, 1.0);
        rightSliders[i].setColour(juce::Slider::thumbColourId, juce::Colour(0xFFE53E3E));
        rightSliders[i].setColour(juce::Slider::trackColourId, juce::Colour(0xFFC53030));
        addAndMakeVisible(rightSliders[i]);

        leftSliders[i].onValueChange = [this, i] { updateParam(i, true); };
        rightSliders[i].onValueChange = [this, i] { updateParam(i, false); };

        freqLabels[i].setText(freqs[i] + " Hz", juce::dontSendNotification);
        freqLabels[i].setJustificationType(juce::Justification::centred);
        freqLabels[i].setColour(juce::Label::textColourId, juce::Colour(0xFFA0AEC0));
        addAndMakeVisible(freqLabels[i]);
    }

    simButton.setButtonText("Simulation Mode (Inverse Loss)");
    simButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    simButton.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFFED8936));
    simButton.onClick = [this] {
        if (auto* param = processor.apvts.getParameter("SIM_MODE"))
            param->setValueNotifyingHost(simButton.getToggleState() ? 1.0f : 0.0f);
    };
    addAndMakeVisible(simButton);

    setSize(640, 360);
}

void AudiogramView::updateParam(int index, bool isLeft)
{
    juce::String id = (isLeft ? "L_GAIN" : "R_GAIN") + juce::String(index);
    float val = isLeft ? static_cast<float>(leftSliders[index].getValue())
                       : static_cast<float>(rightSliders[index].getValue());
    if (auto* param = processor.apvts.getParameter(id))
        param->setValueNotifyingHost(val / 100.0f); // Normalize 0-100 to 0.0-1.0
}

void AudiogramView::paint(juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient bgGradient(juce::Colour(0xFF1A202C), 0.0f, 0.0f,
                                    juce::Colour(0xFF171923), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // App Header Banner
    g.setColour(juce::Colour(0xFF2D3748));
    g.fillRoundedRectangle(15.0f, 10.0f, static_cast<float>(getWidth() - 30), 45.0f, 6.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("HEARING ASSIST - PERSONALIZED AUDIO AMPLIFIER", 20, 10, getWidth() - 40, 45, juce::Justification::centred);

    // Channel Indicators
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xFF63B3ED));
    g.drawText("Left Ear (dB HL)", 40, 62, 160, 20, juce::Justification::left);

    g.setColour(juce::Colour(0xFFFC8181));
    g.drawText("Right Ear (dB HL)", getWidth() - 200, 62, 160, 20, juce::Justification::right);
}

void AudiogramView::resized()
{
    int sliderY = 90;
    int sliderH = 180;
    int spacing = 90;
    int startX = 50;

    for (int i = 0; i < 6; ++i)
    {
        leftSliders[i].setBounds(startX + (i * spacing), sliderY, 38, sliderH);
        rightSliders[i].setBounds(startX + (i * spacing) + 40, sliderY, 38, sliderH);
        freqLabels[i].setBounds(startX + (i * spacing), sliderY + sliderH + 4, 80, 20);
    }

    simButton.setBounds(getWidth() - 240, getHeight() - 45, 220, 30);
}
