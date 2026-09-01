# Personal Hearing Assistant

A real-time, low-latency audiological processing application for Windows 10/11 that intercepts system audio (YouTube, Spotify, games, etc.) via WASAPI, applies clinical audiological correction algorithms (NAL-R, NAL-NL2, Half-Gain, POGO), and streams personalized sound to headphones.

---

## 🏛 Repository Structure

```
personalized-hearing-assistant/
│
├── desktop/                  # Native Windows Application (C++ / JUCE / CMake)
│   ├── CMakeLists.txt
│   ├── Source/
│   │   ├── App/              # Standalone Application Launcher
│   │   ├── Audiometry/       # Clinical Threshold Matrix (0-120 dB HL) & Presets
│   │   ├── HearingProfile/   # NAL-R, NAL-NL2, Half-Gain, POGO Fitting Engines
│   │   ├── DSP/              # 6-Band IIR Filter Bank, WDRC & MPO Safety Limiter
│   │   ├── Storage/          # JSON Profile Persistence
│   │   └── UI/               # Interactive Audiogram Graph & Dashboard Editor
│   └── Assets/
│
├── web/                      # Next.js Web Presentation & In-Browser Web Audio Demo
│   ├── src/app/              # Interactive Web Audio DSP Playground
│   └── package.json
│
├── docs/                     # Technical & Audiological Documentation
│   └── ARCHITECTURE.md
│
├── tests/                    # Automated Unit Tests
│
└── README.md
```

---

## 🚀 Native Windows Desktop Application (`PersonalHearingAssistant.exe`)

The desktop application is the primary product that runs on the user's PC with real-time processing and direct Windows system audio loopback capture.

### Build Instructions

#### Prerequisites
- Windows 10 or 11
- Visual Studio 2022+ with **Desktop development with C++**
- CMake 3.22+

#### Build Steps

```powershell
# 1. Clone repository & navigate to desktop directory
cd D:\projects\amplify\desktop

# 2. Generate Visual Studio project files with CMake
mkdir build -Force
cd build
cmake .. -A x64

# 3. Build Standalone Application in Release mode
cmake --build . --config Release --parallel

# 4. Run Executable
.\PersonalHearingAssistant_artefacts\Release\Standalone\PersonalHearingAssistant.exe
```

---

## 🎧 Windows System Audio Routing (YouTube / Spotify)

1. Launch `PersonalHearingAssistant.exe`.
2. Click **Options -> Audio Settings**.
3. Under **Input Device**, select your physical headphones marked **"(Loopback)"** (or select **"CABLE Output"** if using VB-Audio Virtual Cable).
4. Under **Output Device**, select your physical **Headphones**.
5. In the UI:
   - Pick a preset (e.g. *Moderate Presbycusis* or *Noise-Induced Notch*).
   - Select **Personalized Amplification** for real-time hearing correction.
   - Select **Loss Simulation** to hear what someone with hearing loss perceives.
6. Play any video on YouTube: audio flows through the personalized DSP filter bank straight into your headphones.

---

## 🌐 Web Presentation & Live In-Browser Demo (Vercel)

The `web/` directory contains a Next.js 14 web application designed for global deployment on Vercel:

```powershell
cd D:\projects\amplify\web
npm install
npm run dev
```

Visit [http://localhost:3000](http://localhost:3000) to test the Web Audio API DSP simulation live in your browser.

---

## 📄 License
Licensed under the [Apache License, Version 2.0](LICENSE).
