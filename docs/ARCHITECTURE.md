# Personal Hearing Assistant: System Architecture

## Overview
Personal Hearing Assistant is a real-time, low-latency audiological processing application engineered for Windows. It captures system-wide audio (YouTube, Spotify, games, etc.) via WASAPI, passes the stream through an audiological filter bank with personalized prescription curves (NAL-R, NAL-NL2, Half-Gain, POGO), applies wide dynamic range compression and lookahead MPO brickwall limiting, and outputs to headphones.

---

## 1. System Topology

```
┌──────────────────────────────────────────────────────────┐
│                   Windows Applications                   │
│               YouTube / Spotify / VLC / Games            │
└────────────────────────────┬─────────────────────────────┘
                             │
                             ▼
┌──────────────────────────────────────────────────────────┐
│                  Windows Audio Engine                    │
│            (WASAPI Loopback Capture Endpoint)            │
└────────────────────────────┬─────────────────────────────┘
                             │
                             ▼
┌──────────────────────────────────────────────────────────┐
│             PersonalHearingAssistant.exe                 │
│                                                          │
│  ┌────────────────────────────────────────────────────┐  │
│  │ 1. Audiometry Matrix (ThresholdData)               │  │
│  │    • 250 Hz - 8000 Hz, Air/Bone, MCL, UCL, PTA3    │  │
│  └─────────────────────────┬──────────────────────────┘  │
│                            │                             │
│                            ▼                             │
│  ┌────────────────────────────────────────────────────┐  │
│  │ 2. Prescription Engine (FittingAlgorithms)         │  │
│  │    • NAL-R (Speech Intelligibility)                │  │
│  │    • NAL-NL2 (Multi-Level Non-Linear)              │  │
│  │    • Half-Gain & POGO (Low-Cut Targets)            │  │
│  └─────────────────────────┬──────────────────────────┘  │
│                            │                             │
│                            ▼                             │
│  ┌────────────────────────────────────────────────────┐  │
│  │ 3. Real-Time DSP Engine (PersonalizedDSP)          │  │
│  │    • 6-Band IIR Peaking Filter Bank                │  │
│  │    • Independent Left & Right Gains                │  │
│  │    • Dynamic Safety Compressor                     │  │
│  │    • Lookahead MPO Brickwall Limiter               │  │
│  └─────────────────────────┬──────────────────────────┘  │
│                            │                             │
│                            ▼                             │
│  ┌────────────────────────────────────────────────────┐  │
│  │ 4. Auditioning & Simulation Mode                   │  │
│  │    • Bypass (Clean Audio)                          │  │
│  │    • Loss Simulation (Inverse Attenuation)         │  │
│  │    • Personalized Amplification (Prescribed Target)│  │
│  └────────────────────────────────────────────────────┘  │
└────────────────────────────┬─────────────────────────────┘
                             │
                             ▼
┌──────────────────────────────────────────────────────────┐
│                 Physical Audio Endpoint                  │
│                      (Headphones)                        │
└──────────────────────────────────────────────────────────┘
```

---

## 2. Mathematical Prescription Formulas

### NAL-R (National Acoustic Laboratories - Revised)
$$H_{3FA} = \frac{Loss_{500} + Loss_{1000} + Loss_{2000}}{3}$$
$$X = 0.05 \cdot H_{3FA}$$
$$IG(f) = X + 0.31 \cdot Loss(f) + k(f)$$

Where $k(f)$ values are clinical correction constants:
- 250 Hz: $-17\text{ dB}$
- 500 Hz: $-8\text{ dB}$
- 1000 Hz: $+1\text{ dB}$
- 2000 Hz: $-1\text{ dB}$
- 4000 Hz: $-2\text{ dB}$
- 8000 Hz: $-4\text{ dB}$

### Half-Gain & POGO Rules
$$Gain_{HalfGain}(f) = 0.5 \cdot Loss(f)$$
$$Gain_{POGO}(f) = 0.5 \cdot Loss(f) - C(f)$$
Where $C(250) = 10\text{ dB}$, $C(500) = 5\text{ dB}$, and $0\text{ dB}$ for $\ge 1000\text{ Hz}$.

---

## 3. Hearing Safety & Maximum Power Output (MPO)
Hearing amplification must strictly prevent loudness above the patient's Uncomfortable Loudness Level (UCL) to avoid acoustic shock:
- High-speed lookahead limiter ($<2\text{ ms}$ attack) guarantees 0 overshoot beyond the user's MPO ceiling (default $-1.0\text{ dBFS}$ / $\approx 88\text{ dB SPL}$).
