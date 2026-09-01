#include "ProfileStore.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace HearingAssist {

std::string ProfileStore::exportAudiogramToJson(const ThresholdData& data) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"profileName\": \"" << data.profileName << "\",\n";
    ss << "  \"notes\": \"" << data.notes << "\",\n";
    ss << "  \"leftEarLoss\": [\n";

    const auto& freqs = ThresholdData::getStandardFrequencies();
    for (size_t i = 0; i < freqs.size(); ++i) {
        ss << "    {\"freq\": " << freqs[i] << ", \"dbHL\": " << data.getAirThreshold(Ear::Left, freqs[i]) << "}";
        if (i + 1 < freqs.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";

    ss << "  \"rightEarLoss\": [\n";
    for (size_t i = 0; i < freqs.size(); ++i) {
        ss << "    {\"freq\": " << freqs[i] << ", \"dbHL\": " << data.getAirThreshold(Ear::Right, freqs[i]) << "}";
        if (i + 1 < freqs.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";

    return ss.str();
}

bool ProfileStore::importAudiogramFromJson(const std::string& json, ThresholdData& outData) {
    if (json.empty()) return false;

    // Parse profile name
    size_t namePos = json.find("\"profileName\":");
    if (namePos != std::string::npos) {
        size_t start = json.find("\"", namePos + 14);
        if (start != std::string::npos) {
            size_t end = json.find("\"", start + 1);
            if (end != std::string::npos) {
                outData.profileName = json.substr(start + 1, end - start - 1);
            }
        }
    }

    // Parse frequencies & thresholds
    size_t pos = 0;
    while ((pos = json.find("\"freq\":", pos)) != std::string::npos) {
        size_t fStart = pos + 7;
        size_t fEnd = json.find_first_of(",}", fStart);
        float freq = std::stof(json.substr(fStart, fEnd - fStart));

        size_t dbPos = json.find("\"dbHL\":", fEnd);
        if (dbPos != std::string::npos && dbPos - fEnd < 50) {
            size_t dbStart = dbPos + 7;
            size_t dbEnd = json.find_first_of(",}", dbStart);
            float db = std::stof(json.substr(dbStart, dbEnd - dbStart));

            size_t rightSection = json.find("\"rightEarLoss\":");
            if (rightSection != std::string::npos && pos > rightSection) {
                outData.setAirThreshold(Ear::Right, freq, db);
            } else {
                outData.setAirThreshold(Ear::Left, freq, db);
            }
        }
        pos = fEnd;
    }

    return true;
}

bool ProfileStore::saveToFile(const std::string& filepath, const ThresholdData& data) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;
    out << exportAudiogramToJson(data);
    return true;
}

bool ProfileStore::loadFromFile(const std::string& filepath, ThresholdData& outData) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return importAudiogramFromJson(content, outData);
}

} // namespace HearingAssist
