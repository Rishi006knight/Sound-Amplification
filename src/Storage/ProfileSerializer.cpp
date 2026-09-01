#include "ProfileSerializer.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace Amplify {

std::string ProfileSerializer::serializeAudiogramToJson(const AudiogramData& audiogram) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"patientId\": \"" << audiogram.patientId << "\",\n";
    ss << "  \"patientName\": \"" << audiogram.patientName << "\",\n";
    ss << "  \"date\": \"" << audiogram.examinationDate << "\",\n";
    ss << "  \"notes\": \"" << audiogram.notes << "\",\n";

    // Left Ear
    ss << "  \"leftEar\": {\n";
    ss << "    \"airConduction\": [\n";
    const auto& leftAC = audiogram.getEar(Ear::Left).airConduction;
    for (size_t i = 0; i < leftAC.size(); ++i) {
        ss << "      {\"frequency\": " << leftAC[i].frequencyHz
           << ", \"thresholdDb\": " << leftAC[i].thresholdDbHL
           << ", \"mcl\": " << leftAC[i].mclDbHL
           << ", \"ucl\": " << leftAC[i].uclDbHL << "}";
        if (i + 1 < leftAC.size()) ss << ",";
        ss << "\n";
    }
    ss << "    ]\n";
    ss << "  },\n";

    // Right Ear
    ss << "  \"rightEar\": {\n";
    ss << "    \"airConduction\": [\n";
    const auto& rightAC = audiogram.getEar(Ear::Right).airConduction;
    for (size_t i = 0; i < rightAC.size(); ++i) {
        ss << "      {\"frequency\": " << rightAC[i].frequencyHz
           << ", \"thresholdDb\": " << rightAC[i].thresholdDbHL
           << ", \"mcl\": " << rightAC[i].mclDbHL
           << ", \"ucl\": " << rightAC[i].uclDbHL << "}";
        if (i + 1 < rightAC.size()) ss << ",";
        ss << "\n";
    }
    ss << "    ]\n";
    ss << "  }\n";
    ss << "}\n";

    return ss.str();
}

static std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";
    size_t start = json.find("\"", pos + needle.length());
    if (start == std::string::npos) return "";
    size_t end = json.find("\"", start + 1);
    if (end == std::string::npos) return "";
    return json.substr(start + 1, end - start - 1);
}

bool ProfileSerializer::deserializeAudiogramFromJson(const std::string& jsonString, AudiogramData& outAudiogram) {
    if (jsonString.empty()) return false;

    outAudiogram.patientId = extractJsonString(jsonString, "patientId");
    outAudiogram.patientName = extractJsonString(jsonString, "patientName");
    outAudiogram.examinationDate = extractJsonString(jsonString, "date");
    outAudiogram.notes = extractJsonString(jsonString, "notes");

    // Simple parsing of frequency/threshold pairs
    size_t searchPos = 0;
    while ((searchPos = jsonString.find("\"frequency\":", searchPos)) != std::string::npos) {
        size_t fStart = searchPos + 12;
        size_t fEnd = jsonString.find_first_of(",}", fStart);
        float freq = std::stof(jsonString.substr(fStart, fEnd - fStart));

        size_t tPos = jsonString.find("\"thresholdDb\":", fEnd);
        if (tPos != std::string::npos && tPos - fEnd < 100) {
            size_t tStart = tPos + 14;
            size_t tEnd = jsonString.find_first_of(",}", tStart);
            float thresh = std::stof(jsonString.substr(tStart, tEnd - tStart));

            // Determine if in left or right ear section by checking positions
            size_t rightPos = jsonString.find("\"rightEar\":");
            if (rightPos != std::string::npos && searchPos > rightPos) {
                outAudiogram.setThreshold(Ear::Right, freq, thresh);
            } else {
                outAudiogram.setThreshold(Ear::Left, freq, thresh);
            }
        }
        searchPos = fEnd;
    }

    return true;
}

std::string ProfileSerializer::serializeHearingProfileToJson(const HearingProfile& profile) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"formula\": \"" << static_cast<int>(profile.formulaType) << "\",\n";
    ss << "  \"masterGainDb\": " << profile.masterGainDb << ",\n";
    ss << "  \"bassTrimDb\": " << profile.bassTrimDb << ",\n";
    ss << "  \"presenceTrimDb\": " << profile.presenceTrimDb << ",\n";
    ss << "  \"trebleTrimDb\": " << profile.trebleTrimDb << ",\n";
    ss << "  \"safetyLimiterCeiling\": " << profile.safetyLimiterCeilingDbSPL << ",\n";

    ss << "  \"leftEarTargets\": [\n";
    for (size_t i = 0; i < profile.leftEarProfile.bandTargets.size(); ++i) {
        const auto& bt = profile.leftEarProfile.bandTargets[i];
        ss << "    {\"frequency\": " << bt.centerFrequencyHz
           << ", \"gain\": " << bt.gainMediumDb
           << ", \"cr\": " << bt.compressionRatio << "}";
        if (i + 1 < profile.leftEarProfile.bandTargets.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";

    ss << "  \"rightEarTargets\": [\n";
    for (size_t i = 0; i < profile.rightEarProfile.bandTargets.size(); ++i) {
        const auto& bt = profile.rightEarProfile.bandTargets[i];
        ss << "    {\"frequency\": " << bt.centerFrequencyHz
           << ", \"gain\": " << bt.gainMediumDb
           << ", \"cr\": " << bt.compressionRatio << "}";
        if (i + 1 < profile.rightEarProfile.bandTargets.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";

    return ss.str();
}

bool ProfileSerializer::deserializeHearingProfileFromJson(const std::string& jsonString, HearingProfile& outProfile) {
    if (jsonString.empty()) return false;

    try {
        size_t fPos = jsonString.find("\"formula\":");
        if (fPos != std::string::npos) {
            size_t valStart = jsonString.find_first_of("0123456789", fPos);
            if (valStart != std::string::npos) {
                outProfile.formulaType = static_cast<PrescriptionFormulaType>(std::stoi(jsonString.substr(valStart, 1)));
            }
        }

        auto readVal = [&](const std::string& key, float defaultVal) -> float {
            std::string needle = "\"" + key + "\":";
            size_t pos = jsonString.find(needle);
            if (pos == std::string::npos) return defaultVal;
            size_t start = pos + needle.length();
            size_t end = jsonString.find_first_of(",}\n", start);
            return std::stof(jsonString.substr(start, end - start));
        };

        outProfile.masterGainDb = readVal("masterGainDb", 0.0f);
        outProfile.bassTrimDb = readVal("bassTrimDb", 0.0f);
        outProfile.presenceTrimDb = readVal("presenceTrimDb", 0.0f);
        outProfile.trebleTrimDb = readVal("trebleTrimDb", 0.0f);
        outProfile.safetyLimiterCeilingDbSPL = readVal("safetyLimiterCeiling", 88.0f);

        return true;
    } catch (...) {
        return false;
    }
}

bool ProfileSerializer::saveAudiogramToFile(const std::string& filePath, const AudiogramData& audiogram) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;
    out << serializeAudiogramToJson(audiogram);
    return true;
}

bool ProfileSerializer::loadAudiogramFromFile(const std::string& filePath, AudiogramData& outAudiogram) {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return deserializeAudiogramFromJson(content, outAudiogram);
}

bool ProfileSerializer::saveProfileToFile(const std::string& filePath, const HearingProfile& profile) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;
    out << serializeHearingProfileToJson(profile);
    return true;
}

bool ProfileSerializer::loadProfileFromFile(const std::string& filePath, HearingProfile& outProfile) {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return deserializeHearingProfileFromJson(content, outProfile);
}

} // namespace Amplify
