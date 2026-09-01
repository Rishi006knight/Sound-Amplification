#include "AudiogramView.h"
#include "DSP/HearingAssistProcessor.h"
#include "Audiometry/ThresholdData.h"

AudiogramView::AudiogramView(HearingAssistProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    addAndMakeVisible(audiogramGraph);

    const juce::String freqs[6] = { "250", "500", "1k", "2k", "4k", "8k" };
    for (int i = 0; i < 6; ++i)
    {
        leftSliders[i].setSliderStyle(juce::Slider::LinearVertical);
        leftSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);
        leftSliders[i].setRange(0.0, 100.0, 1.0);
        leftSliders[i].setColour(juce::Slider::thumbColourId, juce::Colour(0xFF3182CE));
        leftSliders[i].setColour(juce::Slider::trackColourId, juce::Colour(0xFF2B6CB0));
        addAndMakeVisible(leftSliders[i]);

        rightSliders[i].setSliderStyle(juce::Slider::LinearVertical);
        rightSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);
        rightSliders[i].setRange(0.0, 100.0, 1.0);
        rightSliders[i].setColour(juce::Slider::thumbColourId, juce::Colour(0xFFE53E3E));
        rightSliders[i].setColour(juce::Slider::trackColourId, juce::Colour(0xFFC53030));
        addAndMakeVisible(rightSliders[i]);

        leftSliders[i].onValueChange = [this, i] { updateSliderParam(i, true); };
        rightSliders[i].onValueChange = [this, i] { updateSliderParam(i, false); };

        freqLabels[i].setText(freqs[i], juce::dontSendNotification);
        freqLabels[i].setJustificationType(juce::Justification::centred);
        freqLabels[i].setColour(juce::Label::textColourId, juce::Colour(0xFFA0AEC0));
        freqLabels[i].setFont(11.0f);
        addAndMakeVisible(freqLabels[i]);
    }

    presetBox.addItem("Preset: Normal Hearing", 1);
    presetBox.addItem("Preset: Mild High-Freq Loss", 2);
    presetBox.addItem("Preset: Moderate Presbycusis", 3);
    presetBox.addItem("Preset: Noise-Induced (4k Notch)", 4);
    presetBox.addItem("Preset: Cookie-Bite (Mid-Loss)", 5);
    presetBox.addItem("Preset: Flat Moderate (45 dB)", 6);
    presetBox.addItem("Preset: Asymmetric Loss", 7);
    presetBox.setSelectedId(3, juce::dontSendNotification);
    presetBox.onChange = [this] { selectPreset(presetBox.getSelectedId()); };
    addAndMakeVisible(presetBox);

    formulaBox.addItem("Formula: NAL-R (Speech Optimal)", 1);
    formulaBox.addItem("Formula: NAL-NL2 (Non-Linear)", 2);
    formulaBox.addItem("Formula: Half-Gain (1/2 Loss)", 3);
    formulaBox.addItem("Formula: POGO (Low-Cut)", 4);
    formulaBox.setSelectedId(1, juce::dontSendNotification);
    formulaBox.onChange = [this] {
        if (auto* param = processor.apvts.getParameter("FORMULA"))
            param->setValueNotifyingHost((formulaBox.getSelectedId() - 1) / 3.0f);
    };
    addAndMakeVisible(formulaBox);

    bypassButton.onClick = [this] { updateModeButtons(0); };
    simButton.onClick = [this] { updateModeButtons(1); };
    ampButton.onClick = [this] { updateModeButtons(2); };
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(simButton);
    addAndMakeVisible(ampButton);
    updateModeButtons(2);

    auto setupRotary = [this](juce::Slider& s, juce::Label& l, double min, double max, double def, const juce::String& suffix) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
        s.setRange(min, max, 0.5);
        s.setValue(def);
        s.setTextValueSuffix(suffix);
        addAndMakeVisible(s);
        l.setJustificationType(juce::Justification::centred);
        l.setFont(11.0f);
        l.setColour(juce::Label::textColourId, juce::Colour(0xFFA0AEC0));
        addAndMakeVisible(l);
    };

    setupRotary(masterGainSlider, masterGainLabel, -12.0, 12.0, 0.0, " dB");
    setupRotary(bassTrimSlider, bassTrimLabel, -10.0, 10.0, 0.0, " dB");
    setupRotary(trebleTrimSlider, trebleTrimLabel, -10.0, 10.0, 0.0, " dB");
    setupRotary(limiterCeilingSlider, limiterCeilingLabel, -18.0, -0.5, -1.0, " dBFS");

    masterGainSlider.onValueChange = [this] {
        if (auto* p = processor.apvts.getParameter("MASTER_GAIN"))
            p->setValueNotifyingHost(juce::jmap((float)masterGainSlider.getValue(), -12.0f, 12.0f, 0.0f, 1.0f));
    };

    bassTrimSlider.onValueChange = [this] {
        if (auto* p = processor.apvts.getParameter("BASS_TRIM"))
            p->setValueNotifyingHost(juce::jmap((float)bassTrimSlider.getValue(), -10.0f, 10.0f, 0.0f, 1.0f));
    };

    trebleTrimSlider.onValueChange = [this] {
        if (auto* p = processor.apvts.getParameter("TREBLE_TRIM"))
            p->setValueNotifyingHost(juce::jmap((float)trebleTrimSlider.getValue(), -10.0f, 10.0f, 0.0f, 1.0f));
    };

    limiterCeilingSlider.onValueChange = [this] {
        if (auto* p = processor.apvts.getParameter("LIMITER_CEILING"))
            p->setValueNotifyingHost(juce::jmap((float)limiterCeilingSlider.getValue(), -18.0f, -0.5f, 0.0f, 1.0f));
    };

    vuMeterLabel.setText("Output: -∞ dBFS | Limiter: Ready", juce::dontSendNotification);
    vuMeterLabel.setFont(12.0f);
    vuMeterLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF38A169));
    addAndMakeVisible(vuMeterLabel);

    lossDegreeLabel.setText("Diagnosis: Moderate Loss (Left & Right)", juce::dontSendNotification);
    lossDegreeLabel.setFont(12.0f);
    lossDegreeLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF63B3ED));
    addAndMakeVisible(lossDegreeLabel);

    selectPreset(3);

    setSize(780, 480);
    startTimerHz(25);
}

AudiogramView::~AudiogramView() {
    stopTimer();
}

void AudiogramView::updateSliderParam(int index, bool isLeft)
{
    juce::String id = (isLeft ? "L_GAIN" : "R_GAIN") + juce::String(index);
    float val = isLeft ? static_cast<float>(leftSliders[index].getValue())
                       : static_cast<float>(rightSliders[index].getValue());
    if (auto* param = processor.apvts.getParameter(id))
        param->setValueNotifyingHost(val / 100.0f);

    updateAudiogramGraphFromSliders();
}

void AudiogramView::updateAudiogramGraphFromSliders()
{
    std::array<float, 6> left{}, right{};
    for (int i = 0; i < 6; ++i) {
        left[i] = static_cast<float>(leftSliders[i].getValue());
        right[i] = static_cast<float>(rightSliders[i].getValue());
    }
    audiogramGraph.setThresholds(left, right);
}

void AudiogramView::selectPreset(int presetId)
{
    HearingAssist::ThresholdData data;
    switch (presetId) {
        case 1: data = HearingAssist::ClinicalPresets::createNormalHearing(); break;
        case 2: data = HearingAssist::ClinicalPresets::createMildPresbycusis(); break;
        case 3: data = HearingAssist::ClinicalPresets::createModeratePresbycusis(); break;
        case 4: data = HearingAssist::ClinicalPresets::createNoiseInducedNotch(); break;
        case 5: data = HearingAssist::ClinicalPresets::createCookieBite(); break;
        case 6: data = HearingAssist::ClinicalPresets::createModerateFlatLoss(); break;
        case 7: data = HearingAssist::ClinicalPresets::createAsymmetricLoss(); break;
        default: data = HearingAssist::ClinicalPresets::createModeratePresbycusis(); break;
    }

    const auto& freqs = HearingAssist::ThresholdData::getStandardFrequencies();
    for (int i = 0; i < 6; ++i) {
        leftSliders[i].setValue(data.getAirThreshold(HearingAssist::Ear::Left, freqs[i]), juce::dontSendNotification);
        rightSliders[i].setValue(data.getAirThreshold(HearingAssist::Ear::Right, freqs[i]), juce::dontSendNotification);
        updateSliderParam(i, true);
        updateSliderParam(i, false);
    }
    updateAudiogramGraphFromSliders();

    lossDegreeLabel.setText("Diagnosis: " + data.getLossDegreeString(data.getLossDegree(HearingAssist::Ear::Left)),
                           juce::dontSendNotification);
}

void AudiogramView::updateModeButtons(int activeMode)
{
    if (auto* param = processor.apvts.getParameter("AUDITION_MODE"))
        param->setValueNotifyingHost(activeMode / 2.0f);

    bypassButton.setColour(juce::TextButton::buttonColourId, (activeMode == 0) ? juce::Colour(0xFF718096) : juce::Colour(0xFF2D3748));
    simButton.setColour(juce::TextButton::buttonColourId, (activeMode == 1) ? juce::Colour(0xFFDD6B20) : juce::Colour(0xFF2D3748));
    ampButton.setColour(juce::TextButton::buttonColourId, (activeMode == 2) ? juce::Colour(0xFF2B6CB0) : juce::Colour(0xFF2D3748));
}

void AudiogramView::timerCallback()
{
    auto meters = processor.getDSP().getMeteringInfo();
    juce::String status = "In: " + juce::String(meters.inputPeakDb, 1) + " dB | Out: "
                        + juce::String(meters.outputPeakDb, 1) + " dB"
                        + (meters.isLimitingActive ? " [MPO LIMITING]" : "");
    vuMeterLabel.setText(status, juce::dontSendNotification);
}

void AudiogramView::paint(juce::Graphics& g)
{
    juce::ColourGradient bgGradient(juce::Colour(0xFF141821), 0.0f, 0.0f,
                                    juce::Colour(0xFF0F1218), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    g.setColour(juce::Colour(0xFF1E2430));
    g.fillRoundedRectangle(12.0f, 8.0f, static_cast<float>(getWidth() - 24), 42.0f, 6.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("PERSONAL HEARING ASSISTANT - SYSTEM AUDIO DSP", 20, 8, getWidth() - 40, 42, juce::Justification::left);

    g.setColour(juce::Colour(0xFF2D3748));
    g.drawRoundedRectangle(12.0f, 56.0f, 370.0f, 210.0f, 6.0f, 1.0f);
    g.drawRoundedRectangle(390.0f, 56.0f, static_cast<float>(getWidth() - 402), 210.0f, 6.0f, 1.0f);
    g.drawRoundedRectangle(12.0f, 274.0f, static_cast<float>(getWidth() - 24), 160.0f, 6.0f, 1.0f);

    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xFF63B3ED));
    g.drawText("Left (Blue)", 405, 62, 80, 16, juce::Justification::left);
    g.setColour(juce::Colour(0xFFFC8181));
    g.drawText("Right (Red)", getWidth() - 100, 62, 80, 16, juce::Justification::right);
}

void AudiogramView::resized()
{
    audiogramGraph.setBounds(18, 62, 358, 198);

    int sliderY = 82;
    int sliderH = 150;
    int spacing = 58;
    int startX = 405;

    for (int i = 0; i < 6; ++i) {
        leftSliders[i].setBounds(startX + (i * spacing), sliderY, 26, sliderH);
        rightSliders[i].setBounds(startX + (i * spacing) + 26, sliderY, 26, sliderH);
        freqLabels[i].setBounds(startX + (i * spacing), sliderY + sliderH + 2, 52, 16);
    }

    presetBox.setBounds(20, 285, 230, 28);
    formulaBox.setBounds(260, 285, 230, 28);

    int btnW = 160;
    int btnX = getWidth() - (btnW * 3 + 30);
    bypassButton.setBounds(btnX, 285, btnW - 5, 28);
    simButton.setBounds(btnX + btnW, 285, btnW - 5, 28);
    ampButton.setBounds(btnX + btnW * 2, 285, btnW - 5, 28);

    int rotY = 328;
    int rotW = 85;
    int rotSpacing = 110;
    int rotStartX = 40;

    masterGainSlider.setBounds(rotStartX, rotY, rotW, 70);
    masterGainLabel.setBounds(rotStartX, rotY + 70, rotW, 16);

    bassTrimSlider.setBounds(rotStartX + rotSpacing, rotY, rotW, 70);
    bassTrimLabel.setBounds(rotStartX + rotSpacing, rotY + 70, rotW, 16);

    trebleTrimSlider.setBounds(rotStartX + rotSpacing * 2, rotY, rotW, 70);
    trebleTrimLabel.setBounds(rotStartX + rotSpacing * 2, rotY + 70, rotW, 16);

    limiterCeilingSlider.setBounds(rotStartX + rotSpacing * 3, rotY, rotW, 70);
    limiterCeilingLabel.setBounds(rotStartX + rotSpacing * 3, rotY + 70, rotW, 16);

    lossDegreeLabel.setBounds(20, getHeight() - 32, 350, 20);
    vuMeterLabel.setBounds(getWidth() - 360, getHeight() - 32, 340, 20);
}
