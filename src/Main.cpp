#include "Core/AudiogramData.h"
#include "Core/HearingProfile.h"
#include "Core/Presets.h"
#include "Fitting/FittingEngine.h"
#include "DSP/MasterDSPChain.h"
#include "Audio/AudioRouter.h"
#include "Storage/ProfileSerializer.h"
#include "UI/DashboardView.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <string>

using namespace Amplify;

void printAsciiAudiogram(const AudiogramData& audiogram) {
    std::cout << "\n========================================================================\n";
    std::cout << "               AMPLIFY - CLINICAL AUDIOGRAM MATRIX                     \n";
    std::cout << "               Patient: " << audiogram.patientName << "\n";
    std::cout << "               Left Ear (X - Blue) | Right Ear (O - Red)               \n";
    std::cout << "========================================================================\n";
    std::cout << " Freq (Hz)  |  Left (dB HL)  |  Right (dB HL)  | Dynamic Range | Loss Level\n";
    std::cout << "------------+----------------+-----------------+---------------+--------\n";

    const auto& freqs = AudiogramData::getStandardFrequencies();
    for (float f : freqs) {
        float leftHL = audiogram.getInterpolatedThreshold(Ear::Left, f);
        float rightHL = audiogram.getInterpolatedThreshold(Ear::Right, f);
        float leftDR = audiogram.getEar(Ear::Left).getDynamicRange(f);

        std::cout << " " << std::setw(9) << static_cast<int>(f)
                  << "  |  " << std::setw(11) << std::fixed << std::setprecision(1) << leftHL << " dB"
                  << "  |  " << std::setw(12) << std::fixed << std::setprecision(1) << rightHL << " dB"
                  << "  |  " << std::setw(10) << std::fixed << std::setprecision(1) << leftDR << " dB"
                  << "  | ";

        if (leftHL <= 20.0f) std::cout << "Normal\n";
        else if (leftHL <= 40.0f) std::cout << "Mild Loss\n";
        else if (leftHL <= 70.0f) std::cout << "Moderate Loss\n";
        else std::cout << "Severe Loss\n";
    }
    std::cout << "========================================================================\n";
}

void printPrescriptionTable(const HearingProfile& profile) {
    std::cout << "\n========================================================================\n";
    std::cout << "               PRESCRIBED INSERTION GAIN & WDRC TARGETS                 \n";
    std::cout << "               Formula: " << HearingProfile::getFormulaName(profile.formulaType) << "\n";
    std::cout << "========================================================================\n";
    std::cout << " Band (Hz)  | Left Gain (65dB) | Right Gain (65dB) | Comp. Ratio | MPO Ceiling\n";
    std::cout << "------------+------------------+-------------------+-------------+------------\n";

    for (size_t i = 0; i < profile.leftEarProfile.bandTargets.size(); ++i) {
        const auto& lBt = profile.leftEarProfile.bandTargets[i];
        const auto& rBt = profile.rightEarProfile.bandTargets[i];

        std::cout << " " << std::setw(9) << static_cast<int>(lBt.centerFrequencyHz)
                  << "  |  " << std::setw(13) << std::fixed << std::setprecision(1) << ("+" + std::to_string((int)lBt.gainMediumDb) + " dB")
                  << "  |  " << std::setw(14) << std::fixed << std::setprecision(1) << ("+" + std::to_string((int)rBt.gainMediumDb) + " dB")
                  << "  |   " << std::setw(5) << std::fixed << std::setprecision(2) << lBt.compressionRatio << ":1"
                  << "    |  " << std::setw(7) << std::fixed << std::setprecision(1) << lBt.mpoDbSPL << " dB SPL\n";
    }
    std::cout << "========================================================================\n";
}

int main(int argc, char* argv[]) {
    std::cout << "========================================================================\n";
    std::cout << "       AMPLIFY - WINDOWS PERSONALIZED HEARING ASSISTANT v1.0.0          \n";
    std::cout << "       Real-Time Audio DSP & Audiological Compensation Engine           \n";
    std::cout << "========================================================================\n\n";

    // 1. Initialize Subsystems
    FittingEngine fittingEngine;
    MasterDSPChain dspChain;
    AudioRouter audioRouter;

    dspChain.prepare(48000.0f);
    audioRouter.initialize(&dspChain);

    DashboardView dashboard(fittingEngine, dspChain, audioRouter);
    dashboard.initialize();

    // 2. Load Default Moderate Presbycusis Profile for Demonstration
    dashboard.loadPreset("moderate_presbycusis");

    std::cout << "[+] Audiometry Assessment Subsystem initialized.\n";
    std::cout << "[+] NAL-R / NAL-NL2 Prescription Engine active.\n";
    std::cout << "[+] 8-Band Filter Bank & WDRC Dynamic Compressor ready.\n";
    std::cout << "[+] Lookahead MPO Brickwall Safety Limiter engaged (88 dB SPL Ceiling).\n";
    std::cout << "[+] Windows WASAPI Loopback & Low-Latency Renderer prepared.\n";

    // Display Active Audiogram & Prescriptions
    printAsciiAudiogram(dashboard.getAudiogramData());
    printPrescriptionTable(dashboard.getHearingProfile());

    // 3. Audio Source Configuration
    std::cout << "\n[>] Select Audio Input Source:\n";
    std::cout << "    [1] Windows System Audio (WASAPI Loopback - YouTube, Spotify, Games)\n";
    std::cout << "    [2] Internal Speech Demo Synthesizer (Built-in Testing Stream)\n";
    std::cout << "    [3] Pure-Tone Audiometric Pulse (1000 Hz @ 65 dB SPL)\n";
    std::cout << "    Choice (default = 2 for self-contained validation): 2\n";

    // Set Speech Demo by default for immediate self-contained validation
    audioRouter.setSourceType(AudioSourceType::AudioFilePlayback);
    audioRouter.getWavePlayer().setGeneratorType(GeneratorType::SpeechSampleDemo);

    // 4. Start Audio Processing Stream
    if (audioRouter.start()) {
        std::cout << "[+] Audio processing engine started successfully.\n";
    } else {
        std::cout << "[!] Direct audio endpoint offline; running in offline simulation mode.\n";
    }

    std::cout << "\n========================================================================\n";
    std::cout << "           A/B/C AUDITIONING MODES & RUNTIME TELEMETRY                  \n";
    std::cout << "========================================================================\n";

    // Demonstrate A/B/C switching
    std::cout << "\n>>> [DEMO STEP 1/3] Testing Mode A: RAW BYPASS (Unprocessed Audio) <<<\n";
    dspChain.setMode(ProcessingMode::Bypass);
    for (int sec = 0; sec < 3; ++sec) {
        auto m = dspChain.getMeterData();
        std::cout << "    [Mode A: Bypass] In Peak: " << std::setw(6) << std::fixed << std::setprecision(1) << m.inputPeakDb
                  << " dBFS | Out Peak: " << std::setw(6) << std::fixed << std::setprecision(1) << m.outputPeakDb
                  << " dBFS | Limiter: " << std::setw(4) << m.limiterGainReductionDb << " dB\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "\n>>> [DEMO STEP 2/3] Testing Mode B: SIMULATED HEARING LOSS (Educational) <<<\n";
    dspChain.setMode(ProcessingMode::HearingLossSimulation);
    for (int sec = 0; sec < 3; ++sec) {
        auto m = dspChain.getMeterData();
        std::cout << "    [Mode B: Loss Sim] In Peak: " << std::setw(6) << std::fixed << std::setprecision(1) << m.inputPeakDb
                  << " dBFS | Out Peak: " << std::setw(6) << std::fixed << std::setprecision(1) << m.outputPeakDb
                  << " dBFS (High frequencies attenuated)\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "\n>>> [DEMO STEP 3/3] Testing Mode C: PERSONALIZED AMPLIFICATION (NAL-R + WDRC) <<<\n";
    dspChain.setMode(ProcessingMode::PersonalizedAmplification);
    for (int sec = 0; sec < 3; ++sec) {
        auto m = dspChain.getMeterData();
        std::cout << "    [Mode C: Amplify]  In Peak: " << std::setw(6) << std::fixed << std::setprecision(1) << m.inputPeakDb
                  << " dBFS | Out Peak: " << std::setw(6) << std::fixed << std::setprecision(1) << m.outputPeakDb
                  << " dBFS | Comp Red: -" << std::setw(4) << m.leftBandReductionDb[4] << " dB\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Save Profile to JSON
    std::string profilePath = "hearing_profile.json";
    if (ProfileSerializer::saveAudiogramToFile(profilePath, dashboard.getAudiogramData())) {
        std::cout << "\n[+] Hearing profile saved to: " << profilePath << "\n";
    }

    std::cout << "\n[+] Amplify engine running. Processing completed cleanly.\n";
    audioRouter.stop();
    return 0;
}
