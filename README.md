# Amplify: Windows Personalized Hearing Amplification & Simulation System

**Amplify** is a high-performance, real-time audio processing system designed for Windows. It captures system-wide audio (YouTube, Spotify, VLC, games) via Windows WASAPI Loopback, applies clinical audiological correction algorithms (NAL-R, NAL-NL2, Half-Gain, POGO), performs multi-band Wide Dynamic Range Compression (WDRC), guarantees hearing safety through a brickwall MPO limiter, and plays back enhanced audio with near-zero latency.

---

## 🏛 System Architecture & Modules

```
                        ┌────────────────────────┐
                        │        USER            │
                        └───────────┬────────────┘
                                    │
                                    ▼
       ┌────────────────────────────────────────────────────────┐
       │ MODULE 1: AUDIOMETRY & CLINICAL ASSESSMENT             │
       │ • Pure-tone thresholds (125 Hz - 8000 Hz, Air/Bone)     │
       │ • Left (X / Blue) & Right (O / Red) Ear Isolation      │
       │ • Dynamic Range, MCL (Most Comfortable), UCL (Unsafe)  │
       └────────────────────────────┬───────────────────────────┘
                                    │
                                    ▼
       ┌────────────────────────────────────────────────────────┐
       │ MODULE 2: HEARING PROFILE & TARGET PRESCRIPTION        │
       │ • NAL-R (Speech Intelligibility Optimization)          │
       │ • NAL-NL2 (Multi-Level Non-Linear Compression)         │
       │ • Half-Gain Rule & POGO (Low-Cut Speech Target)        │
       │ • Band Compression Ratios & Insertion Gains            │
       └──────────────┬──────────────────────────┬──────────────┘
                      │                          │
                      ▼                          ▼
       ┌────────────────────────┐ ┌─────────────────────────────┐
       │ MODULE 3: REAL-TIME    │ │ MODULE 5: AUDIOMETRY &      │
       │ PERSONALIZED DSP       │ │ HEARING-LOSS SIMULATION     │
       │ • 8-Band Filter Bank   │ │ • Sensorineural loss filter │
       │ • WDRC Compressors     │ │ • Dynamic Range Shrinkage   │
       │ • Lookahead MPO Limiter│ │ • Auditory Recruitment Sim  │
       └──────────────┬─────────┘ └──────────────┬──────────────┘
                      │                          │
                      └─────────────┬────────────┘
                                    │
                                    ▼
       ┌────────────────────────────────────────────────────────┐
       │ MODULE 4: WINDOWS SYSTEM AUDIO LAYER                   │
       │ • WASAPI Loopback Capture (YouTube, Spotify, Games)    │
       │ • Lock-Free Ring Buffering & Low-Latency Rendering     │
       │ • Built-In Signal & Speech Synthesizer Testing Stream  │
       └────────────────────────────┬───────────────────────────┘
                                    │
                                    ▼
                          ┌──────────────────┐
                          │   HEADPHONES     │
                          │   (USER HEARS)   │
                          └──────────────────┘
```

---

## 🚀 Key Features

1. **Clinical Audiometry Matrix**: Full standard audiometric octaves (125 Hz to 8000 Hz) with log-frequency interpolation, Pure Tone Average (PTA3), and automatic loss categorization (Normal, Mild, Moderate, Severe).
2. **Gold-Standard Prescription Engines**:
   - **NAL-R**: Linear prescription with clinical slope factors $k(f)$.
   - **NAL-NL2**: Non-linear level-dependent gain curves for 50 dB SPL (soft speech), 65 dB SPL (conversational speech), and 80 dB SPL (loud sounds).
   - **POGO & Half-Gain**: Standard 50% loss target with 250 Hz and 500 Hz low-cut filters.
3. **8-Band WDRC DSP Chain**:
   - 8-band Linkwitz-Riley crossover filter bank.
   - Independent Wide Dynamic Range Compression (WDRC) per band with soft knee, fast attack, and smooth release envelope followers.
   - Lookahead MPO (Maximum Power Output) brickwall safety limiter clamping output to safe thresholds (85–90 dB SPL).
4. **Educational A/B/C Auditioning Mode**:
   - **Mode A (Bypass)**: Unprocessed original audio.
   - **Mode B (Loss Simulation)**: Simulates the exact hearing loss profile and recruitment for demonstration and research.
   - **Mode C (Amplify)**: Active real-time personalized enhancement.
5. **Windows WASAPI Engine**: High-performance loopback capture from default Windows audio endpoints without requiring driver re-installations.

---

## 📂 Project Directory Layout

```
amplify/
├── CMakeLists.txt              # Top-level build configuration
├── README.md                   # System documentation
├── src/
│   ├── Main.cpp                # Application entry point & demo harness
│   ├── Core/                   # Data models & Presets
│   │   ├── AudiogramData.h/.cpp
│   │   ├── HearingProfile.h/.cpp
│   │   └── Presets.h/.cpp
│   ├── Fitting/                # Prescription Algorithms
│   │   ├── IFittingFormula.h
│   │   ├── NALRFormula.h/.cpp
│   │   ├── HalfGainFormula.h/.cpp
│   │   ├── NALNL2Estimator.h/.cpp
│   │   └── FittingEngine.h/.cpp
│   ├── DSP/                    # Real-Time Audio DSP Engine
│   │   ├── AudioBufferUtils.h
│   │   ├── FilterBank.h/.cpp
│   │   ├── BandCompressor.h/.cpp
│   │   ├── SafetyLimiter.h/.cpp
│   │   ├── HearingLossSimulator.h/.cpp
│   │   └── MasterDSPChain.h/.cpp
│   ├── Audio/                  # Windows WASAPI & Routing
│   │   ├── AudioTypes.h
│   │   ├── WASAPILoopback.h/.cpp
│   │   ├── WASAPIRenderer.h/.cpp
│   │   ├── AudioRouter.h/.cpp
│   │   └── WavePlayer.h/.cpp
│   ├── Storage/                # JSON Persistence
│   │   └── ProfileSerializer.h/.cpp
│   └── UI/                     # UI Views & Components
│       ├── AudiogramView.h/.cpp
│       ├── FittingView.h/.cpp
│       ├── DSPControlView.h/.cpp
│       ├── SimulationView.h/.cpp
│       ├── AudioControlView.h/.cpp
│       ├── SpectrumVisualizer.h/.cpp
│       └── DashboardView.h/.cpp
└── tests/                      # Automated Unit Test Suite
    ├── TestMain.cpp
    ├── TestFittingFormulas.cpp
    ├── TestDSPChain.cpp
    └── TestProfileSerializer.cpp
```

---

## 🛠 Building & Running

### Requirements
- **OS**: Windows 10 / 11
- **Compiler**: Visual Studio 2022+ (MSVC `cl.exe`) or Clang / GCC with C++20 support
- **Build Tool**: CMake 3.20+

### Build Steps

```bash
# 1. Create build directory
mkdir build
cd build

# 2. Configure with CMake (Generates Visual Studio solution or Ninja build)
cmake ..

# 3. Build project
cmake --build . --config Release

# 4. Run Automated Test Suite
ctest --output-on-failure
# Or run directly:
./tests/Release/AmplifyTests.exe

# 5. Launch Application
./Release/AmplifyApp.exe
```

---

## 📊 Preloaded Clinical Presets
- **Normal Hearing (Baseline)**: 0–5 dB HL baseline.
- **Mild High-Frequency Loss**: Age-related early-stage presbycusis.
- **Moderate Sloping Presbycusis**: Classic high-frequency steep slope (15 dB @ 250 Hz to 80 dB @ 8000 Hz).
- **Noise-Induced Notch (4 kHz)**: Acoustic trauma notch at 3–4 kHz.
- **Cookie-Bite**: Mid-frequency genetic loss affecting speech frequencies.
- **Moderate Flat Loss**: Uniform 45 dB HL sensorineural impairment.
- **Asymmetric Loss**: Unilateral loss profile.
