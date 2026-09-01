#include "AudiogramView.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace Amplify {

AudiogramView::AudiogramView() {
    setBounds(0, 0, 600, 400);
}

void AudiogramView::setAudiogramData(const AudiogramData& data) {
    audiogramData = data;
}

void AudiogramView::setPointThreshold(Ear ear, float freqHz, float dbHL) {
    audiogramData.setThreshold(ear, freqHz, dbHL);
    if (onAudiogramChanged) onAudiogramChanged(audiogramData);
}

void AudiogramView::setBounds(int x, int y, int width, int height) {
    viewX = x;
    viewY = y;
    viewWidth = width;
    viewHeight = height;

    gridLeft = x + 60;
    gridRight = x + width - 30;
    gridTop = y + 40;
    gridBottom = y + height - 40;
}

int AudiogramView::freqToScreenX(float freqHz) const {
    float minF = 125.0f;
    float maxF = 8000.0f;
    float logMin = std::log2(minF);
    float logMax = std::log2(maxF);
    float t = (std::log2(std::clamp(freqHz, minF, maxF)) - logMin) / (logMax - logMin);
    return static_cast<int>(gridLeft + t * (gridRight - gridLeft));
}

float AudiogramView::screenXToFreq(int screenX) const {
    float t = static_cast<float>(screenX - gridLeft) / static_cast<float>(gridRight - gridLeft);
    t = std::clamp(t, 0.0f, 1.0f);
    float logMin = std::log2(125.0f);
    float logMax = std::log2(8000.0f);
    return std::pow(2.0f, logMin + t * (logMax - logMin));
}

int AudiogramView::dbHLToScreenY(float dbHL) const {
    // In clinical audiograms, -10 dB HL is at the top, +120 dB HL is at the bottom
    float minDb = -10.0f;
    float maxDb = 120.0f;
    float t = (dbHL - minDb) / (maxDb - minDb);
    return static_cast<int>(gridTop + t * (gridBottom - gridTop));
}

float AudiogramView::screenYToDbHL(int screenY) const {
    float t = static_cast<float>(screenY - gridTop) / static_cast<float>(gridBottom - gridTop);
    t = std::clamp(t, 0.0f, 1.0f);
    return -10.0f + t * 130.0f;
}

bool AudiogramView::handleMouseDown(int mouseX, int mouseY, Ear targetEar) {
    const auto& pts = audiogramData.getEar(targetEar).airConduction;
    for (size_t i = 0; i < pts.size(); ++i) {
        int ptX = freqToScreenX(pts[i].frequencyHz);
        int ptY = dbHLToScreenY(pts[i].thresholdDbHL);

        int dx = mouseX - ptX;
        int dy = mouseY - ptY;
        if (dx * dx + dy * dy <= 120) { // Click radius
            dragState.isDragging = true;
            dragState.draggedEar = targetEar;
            dragState.pointIndex = i;
            return true;
        }
    }
    return false;
}

bool AudiogramView::handleMouseDrag(int /*mouseX*/, int mouseY) {
    if (!dragState.isDragging) return false;

    float rawDb = screenYToDbHL(mouseY);
    // Snap to 5 dB increments (standard clinical audiometry step)
    float snappedDb = std::round(rawDb / 5.0f) * 5.0f;
    snappedDb = std::clamp(snappedDb, -10.0f, 120.0f);

    auto& pts = audiogramData.getEar(dragState.draggedEar).airConduction;
    if (dragState.pointIndex < pts.size()) {
        pts[dragState.pointIndex].thresholdDbHL = snappedDb;
        if (onAudiogramChanged) onAudiogramChanged(audiogramData);
        return true;
    }
    return false;
}

void AudiogramView::handleMouseUp() {
    dragState.isDragging = false;
}

std::vector<AudiogramView::DrawCommand> AudiogramView::generateDrawCommands() const {
    std::vector<DrawCommand> cmds;

    // 1. Shaded Normal Hearing Zone (-10 dB to 20 dB HL)
    int yNormalTop = dbHLToScreenY(-10.0f);
    int yNormalBottom = dbHLToScreenY(20.0f);
    cmds.push_back({DrawCommand::Type::RectShaded, (float)gridLeft, (float)yNormalTop,
                    (float)gridRight, (float)yNormalBottom, 0x1800FF88, "", 0.0f});

    // 2. Horizontal dB HL Grid Lines
    for (int db = -10; db <= 120; db += 10) {
        int y = dbHLToScreenY((float)db);
        unsigned int color = (db == 0 || db == 20) ? 0xFF4A5568 : 0xFF2D3748;
        cmds.push_back({DrawCommand::Type::Line, (float)gridLeft, (float)y, (float)gridRight, (float)y, color, "", 1.0f});

        // dB HL Label on left axis
        cmds.push_back({DrawCommand::Type::Text, (float)(gridLeft - 35), (float)(y + 4), 0, 0, 0xFFA0AEC0, std::to_string(db), 0});
    }

    // 3. Vertical Frequency Grid Lines
    const auto& freqs = AudiogramData::getStandardFrequencies();
    for (float f : freqs) {
        int x = freqToScreenX(f);
        cmds.push_back({DrawCommand::Type::Line, (float)x, (float)gridTop, (float)x, (float)gridBottom, 0xFF2D3748, "", 1.0f});

        // Frequency Label at top axis
        std::string fText = (f >= 1000.0f) ? (std::to_string((int)(f / 1000.0f)) + ((f == 1500.0f || f == 750.0f) ? ".5k" : "k")) : std::to_string((int)f);
        cmds.push_back({DrawCommand::Type::Text, (float)(x - 10), (float)(gridTop - 12), 0, 0, 0xFFA0AEC0, fText, 0});
    }

    // 4. Draw Left Ear Curve (Blue line with 'X' markers)
    const auto& leftPts = audiogramData.getEar(Ear::Left).airConduction;
    for (size_t i = 0; i < leftPts.size(); ++i) {
        int x = freqToScreenX(leftPts[i].frequencyHz);
        int y = dbHLToScreenY(leftPts[i].thresholdDbHL);

        if (i + 1 < leftPts.size()) {
            int nextX = freqToScreenX(leftPts[i + 1].frequencyHz);
            int nextY = dbHLToScreenY(leftPts[i + 1].thresholdDbHL);
            cmds.push_back({DrawCommand::Type::Line, (float)x, (float)y, (float)nextX, (float)nextY, 0xFF3182CE, "", 2.0f});
        }

        // 'X' Marker for Left ear
        cmds.push_back({DrawCommand::Type::Cross, (float)x, (float)y, 0, 0, 0xFF63B3ED, "X", 2.0f});
    }

    // 5. Draw Right Ear Curve (Red line with 'O' circles)
    const auto& rightPts = audiogramData.getEar(Ear::Right).airConduction;
    for (size_t i = 0; i < rightPts.size(); ++i) {
        int x = freqToScreenX(rightPts[i].frequencyHz);
        int y = dbHLToScreenY(rightPts[i].thresholdDbHL);

        if (i + 1 < rightPts.size()) {
            int nextX = freqToScreenX(rightPts[i + 1].frequencyHz);
            int nextY = dbHLToScreenY(rightPts[i + 1].thresholdDbHL);
            cmds.push_back({DrawCommand::Type::Line, (float)x, (float)y, (float)nextX, (float)nextY, 0xFFE53E3E, "", 2.0f});
        }

        // 'O' Circle for Right ear
        cmds.push_back({DrawCommand::Type::Circle, (float)x, (float)y, 5.0f, 0, 0xFFFC8181, "O", 2.0f});
    }

    return cmds;
}

} // namespace Amplify
