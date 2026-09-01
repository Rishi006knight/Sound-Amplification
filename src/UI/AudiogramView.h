#pragma once

#include "Core/AudiogramData.h"
#include <vector>
#include <string>
#include <functional>

namespace Amplify {

struct AudiogramPointDragState {
    bool isDragging{false};
    Ear draggedEar{Ear::Left};
    size_t pointIndex{0};
};

class AudiogramView {
public:
    AudiogramView();
    ~AudiogramView() = default;

    void setAudiogramData(const AudiogramData& data);
    const AudiogramData& getAudiogramData() const { return audiogramData; }

    void setPointThreshold(Ear ear, float frequencyHz, float dbHL);

    // Grid dimension settings
    void setBounds(int x, int y, int width, int height);

    // Coordinate conversions (Screen X/Y <-> Log Frequency / dB HL)
    int freqToScreenX(float freqHz) const;
    float screenXToFreq(int screenX) const;

    int dbHLToScreenY(float dbHL) const;
    float screenYToDbHL(int screenY) const;

    // Mouse interactions for dragging audiogram points
    bool handleMouseDown(int mouseX, int mouseY, Ear targetEar);
    bool handleMouseDrag(int mouseX, int mouseY);
    void handleMouseUp();

    // Callback when user modifies audiogram points
    void setOnAudiogramChanged(std::function<void(const AudiogramData&)> callback) {
        onAudiogramChanged = callback;
    }

    // Render data structure for UI engines (JUCE / GDI+ / Direct2D / Web Canvas)
    struct DrawCommand {
        enum class Type { Line, Circle, Cross, Text, RectShaded };
        Type type;
        float x1, y1, x2, y2;
        unsigned int color; // ARGB
        std::string text;
        float strokeWidth;
    };

    std::vector<DrawCommand> generateDrawCommands() const;

private:
    AudiogramData audiogramData;
    int viewX{0}, viewY{0}, viewWidth{600}, viewHeight{400};
    int gridLeft{60}, gridRight{570}, gridTop{40}, gridBottom{360};

    AudiogramPointDragState dragState;
    std::function<void(const AudiogramData&)> onAudiogramChanged{nullptr};
};

} // namespace Amplify
