#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace HearingAssist {

class AudiogramComponent : public juce::Component {
public:
    AudiogramComponent();
    ~AudiogramComponent() override = default;

    void setThresholds(const std::array<float, 6>& leftDb, const std::array<float, 6>& rightDb);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    std::array<float, 6> leftThresholds{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::array<float, 6> rightThresholds{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    int gridLeft{40}, gridRight{360}, gridTop{25}, gridBottom{200};

    int freqToScreenX(size_t index) const;
    int dbToScreenY(float dbHL) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudiogramComponent)
};

} // namespace HearingAssist
