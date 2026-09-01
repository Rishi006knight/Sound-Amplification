#include "AudiogramComponent.h"

namespace HearingAssist {

AudiogramComponent::AudiogramComponent() {}

void AudiogramComponent::setThresholds(const std::array<float, 6>& leftDb, const std::array<float, 6>& rightDb) {
    leftThresholds = leftDb;
    rightThresholds = rightDb;
    repaint();
}

void AudiogramComponent::resized() {
    gridLeft = 45;
    gridRight = getWidth() - 25;
    gridTop = 25;
    gridBottom = getHeight() - 30;
}

int AudiogramComponent::freqToScreenX(size_t index) const {
    float t = static_cast<float>(index) / 5.0f;
    return static_cast<int>(gridLeft + t * (gridRight - gridLeft));
}

int AudiogramComponent::dbToScreenY(float dbHL) const {
    float t = std::clamp(dbHL, 0.0f, 100.0f) / 100.0f;
    return static_cast<int>(gridTop + t * (gridBottom - gridTop));
}

void AudiogramComponent::paint(juce::Graphics& g) {
    g.setColour(juce::Colour(0xFF1E2430));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    int yNormTop = dbToScreenY(0.0f);
    int yNormBottom = dbToScreenY(20.0f);
    g.setColour(juce::Colour(0x1800E5FF));
    g.fillRect(gridLeft, yNormTop, gridRight - gridLeft, yNormBottom - yNormTop);

    for (int db = 0; db <= 100; db += 20) {
        int y = dbToScreenY(static_cast<float>(db));
        g.setColour(juce::Colour(0xFF2D3748));
        g.drawHorizontalLine(y, static_cast<float>(gridLeft), static_cast<float>(gridRight));

        g.setColour(juce::Colour(0xFFA0AEC0));
        g.setFont(10.0f);
        g.drawText(juce::String(db) + " dB", 5, y - 6, 35, 12, juce::Justification::right);
    }

    const juce::String freqs[6] = { "250", "500", "1k", "2k", "4k", "8k" };
    for (size_t i = 0; i < 6; ++i) {
        int x = freqToScreenX(i);
        g.setColour(juce::Colour(0xFF2D3748));
        g.drawVerticalLine(x, static_cast<float>(gridTop), static_cast<float>(gridBottom));

        g.setColour(juce::Colour(0xFFA0AEC0));
        g.setFont(11.0f);
        g.drawText(freqs[i], x - 15, gridBottom + 4, 30, 16, juce::Justification::centred);
    }

    // Left Ear Curve (Blue)
    juce::Path leftPath;
    for (size_t i = 0; i < 6; ++i) {
        int x = freqToScreenX(i);
        int y = dbToScreenY(leftThresholds[i]);
        if (i == 0) leftPath.startNewSubPath(static_cast<float>(x), static_cast<float>(y));
        else leftPath.lineTo(static_cast<float>(x), static_cast<float>(y));
    }
    g.setColour(juce::Colour(0xFF3182CE));
    g.strokePath(leftPath, juce::PathStrokeType(2.5f));

    for (size_t i = 0; i < 6; ++i) {
        int x = freqToScreenX(i);
        int y = dbToScreenY(leftThresholds[i]);
        g.setColour(juce::Colour(0xFF63B3ED));
        g.drawLine(static_cast<float>(x - 4), static_cast<float>(y - 4), static_cast<float>(x + 4), static_cast<float>(y + 4), 2.0f);
        g.drawLine(static_cast<float>(x - 4), static_cast<float>(y + 4), static_cast<float>(x + 4), static_cast<float>(y - 4), 2.0f);
    }

    // Right Ear Curve (Red)
    juce::Path rightPath;
    for (size_t i = 0; i < 6; ++i) {
        int x = freqToScreenX(i);
        int y = dbToScreenY(rightThresholds[i]);
        if (i == 0) rightPath.startNewSubPath(static_cast<float>(x), static_cast<float>(y));
        else rightPath.lineTo(static_cast<float>(x), static_cast<float>(y));
    }
    g.setColour(juce::Colour(0xFFE53E3E));
    g.strokePath(rightPath, juce::PathStrokeType(2.5f));

    for (size_t i = 0; i < 6; ++i) {
        int x = freqToScreenX(i);
        int y = dbToScreenY(rightThresholds[i]);
        g.setColour(juce::Colour(0xFFFC8181));
        g.drawEllipse(static_cast<float>(x - 4), static_cast<float>(y - 4), 8.0f, 8.0f, 2.0f);
    }
}

} // namespace HearingAssist
