"use client";

import React, { useState, useEffect, useRef } from "react";
import {
  Volume2,
  VolumeX,
  Sliders,
  Activity,
  Headphones,
  ShieldCheck,
  Zap,
  Download,
  Github,
  Play,
  Square,
  Sparkles,
  Info,
  CheckCircle2,
  Layers,
  BarChart3,
} from "lucide-react";

// Standard audiometric octaves
const FREQUENCIES = [250, 500, 1000, 2000, 4000, 8000];

// Clinical Presets
const PRESETS = [
  {
    id: "moderate_presbycusis",
    name: "Moderate Sloping Presbycusis",
    desc: "Age-related high-frequency loss affecting speech consonants.",
    left: [15, 25, 35, 55, 70, 75],
    right: [15, 25, 35, 55, 70, 75],
  },
  {
    id: "mild_presbycusis",
    name: "Mild High-Frequency Loss",
    desc: "Early-stage presbycusis with subtle high-end roll-off.",
    left: [10, 15, 20, 35, 45, 55],
    right: [10, 15, 20, 35, 45, 55],
  },
  {
    id: "noise_notch",
    name: "Noise-Induced Notch (4 kHz)",
    desc: "Acoustic trauma notch at 4000 Hz with recovery at 8000 Hz.",
    left: [10, 15, 20, 35, 65, 30],
    right: [10, 15, 20, 35, 65, 30],
  },
  {
    id: "cookie_bite",
    name: "Cookie-Bite (Mid-Frequency)",
    desc: "Genetic mid-frequency speech dip (500 Hz - 2 kHz).",
    left: [15, 45, 60, 45, 25, 15],
    right: [15, 45, 60, 45, 25, 15],
  },
  {
    id: "flat_loss",
    name: "Flat Moderate Loss (45 dB)",
    desc: "Even 45 dB loss across all frequencies.",
    left: [45, 45, 45, 45, 45, 45],
    right: [45, 45, 45, 45, 45, 45],
  },
  {
    id: "normal",
    name: "Normal Hearing (Baseline)",
    desc: "Baseline audiogram with no hearing impairment.",
    left: [5, 5, 5, 5, 5, 5],
    right: [5, 5, 5, 5, 5, 5],
  },
];

export default function Home() {
  const [selectedPreset, setSelectedPreset] = useState("moderate_presbycusis");
  const [leftLoss, setLeftLoss] = useState<number[]>([15, 25, 35, 55, 70, 75]);
  const [rightLoss, setRightLoss] = useState<number[]>([15, 25, 35, 55, 70, 75]);
  
  // Audition Mode: "bypass" | "simulation" | "amplification"
  const [auditionMode, setAuditionMode] = useState<"bypass" | "simulation" | "amplification">("amplification");
  const [formula, setFormula] = useState<"nal_r" | "half_gain" | "pogo">("nal_r");
  
  // Audio playback state
  const [isPlaying, setIsPlaying] = useState(false);
  const [soundType, setSoundType] = useState<"speech" | "tone" | "pink">("speech");

  // Web Audio Refs
  const audioCtxRef = useRef<AudioContext | null>(null);
  const filtersRef = useRef<BiquadFilterNode[]>([]);
  const sourceNodeRef = useRef<AudioNode | null>(null);
  const gainNodeRef = useRef<GainNode | null>(null);

  // Load preset
  const handlePresetChange = (presetId: string) => {
    setSelectedPreset(presetId);
    const p = PRESETS.find((x) => x.id === presetId);
    if (p) {
      setLeftLoss([...p.left]);
      setRightLoss([...p.right]);
    }
  };

  // Calculate NAL-R or Half-Gain prescribed insertion gain
  const calculateGain = (lossDb: number, freq: number) => {
    if (auditionMode === "bypass") return 0;
    if (auditionMode === "simulation") {
      // Simulation mode: negative gain to simulate sound attenuation
      return -Math.min(lossDb, 65);
    }

    if (formula === "half_gain") {
      return Math.min(30, lossDb * 0.5);
    } else if (formula === "pogo") {
      let g = lossDb * 0.5;
      if (freq <= 250) g -= 10;
      else if (freq <= 500) g -= 5;
      return Math.max(0, Math.min(30, g));
    } else {
      // NAL-R approximation
      const kFactors: Record<number, number> = {
        250: -17,
        500: -8,
        1000: 1,
        2000: -1,
        4000: -2,
        8000: -4,
      };
      const pta3 = (leftLoss[1] + leftLoss[2] + leftLoss[3]) / 3;
      const x = 0.05 * pta3;
      const ig = x + 0.31 * lossDb + (kFactors[freq] || 0);
      return Math.max(0, Math.min(32, ig));
    }
  };

  // Setup / Update Web Audio DSP filters in real-time
  useEffect(() => {
    if (!audioCtxRef.current || filtersRef.current.length === 0) return;

    filtersRef.current.forEach((filter, index) => {
      const f = FREQUENCIES[index];
      const loss = (leftLoss[index] + rightLoss[index]) / 2;
      const gain = calculateGain(loss, f);
      filter.gain.setTargetAtTime(gain, audioCtxRef.current!.currentTime, 0.05);
    });
  }, [leftLoss, rightLoss, auditionMode, formula]);

  // Audio Playback Toggle
  const toggleAudio = async () => {
    if (isPlaying) {
      if (audioCtxRef.current) {
        audioCtxRef.current.close();
        audioCtxRef.current = null;
      }
      setIsPlaying(false);
      return;
    }

    try {
      const AudioContextClass = window.AudioContext || (window as unknown as { webkitAudioContext: typeof AudioContext }).webkitAudioContext;
      const ctx = new AudioContextClass();
      audioCtxRef.current = ctx;

      // Master output limiter gain
      const masterGain = ctx.createGain();
      masterGain.gain.value = 0.3;
      gainNodeRef.current = masterGain;

      // Create 6 peaking IIR filters
      const filters: BiquadFilterNode[] = [];
      FREQUENCIES.forEach((freq, idx) => {
        const filter = ctx.createBiquadFilter();
        filter.type = "peaking";
        filter.frequency.value = freq;
        filter.Q.value = 1.4;
        const loss = (leftLoss[idx] + rightLoss[idx]) / 2;
        filter.gain.value = calculateGain(loss, freq);
        filters.push(filter);
      });
      filtersRef.current = filters;

      // Chain filters sequentially
      for (let i = 0; i < filters.length - 1; i++) {
        filters[i].connect(filters[i + 1]);
      }
      filters[filters.length - 1].connect(masterGain);
      masterGain.connect(ctx.destination);

      // Create Audio Source
      if (soundType === "speech") {
        // Synthesize rich harmonic speech spectrum
        const osc1 = ctx.createOscillator();
        const osc2 = ctx.createOscillator();
        const osc3 = ctx.createOscillator();
        const osc4 = ctx.createOscillator();

        osc1.frequency.value = 130; // Fundamental
        osc2.frequency.value = 700; // F1 vowel
        osc3.frequency.value = 1900; // F2 vowel
        osc4.frequency.value = 3200; // F3 consonant clarity

        const synthGain = ctx.createGain();
        synthGain.gain.value = 0.25;

        osc1.connect(synthGain);
        osc2.connect(synthGain);
        osc3.connect(synthGain);
        osc4.connect(synthGain);

        synthGain.connect(filters[0]);

        osc1.start();
        osc2.start();
        osc3.start();
        osc4.start();
        sourceNodeRef.current = synthGain;
      } else if (soundType === "pink") {
        // Pink noise generator buffer
        const bufferSize = ctx.sampleRate * 2;
        const noiseBuffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
        const output = noiseBuffer.getChannelData(0);
        let b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
        for (let i = 0; i < bufferSize; i++) {
          const white = Math.random() * 2 - 1;
          b0 = 0.99886 * b0 + white * 0.0555179;
          b1 = 0.99332 * b1 + white * 0.0750759;
          b2 = 0.96900 * b2 + white * 0.1538520;
          b3 = 0.86650 * b3 + white * 0.3104856;
          b4 = 0.55000 * b4 + white * 0.5329522;
          b5 = -0.7616 * b5 - white * 0.0168980;
          output[i] = (b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362) * 0.08;
          b6 = white * 0.115926;
        }
        const whiteNoise = ctx.createBufferSource();
        whiteNoise.buffer = noiseBuffer;
        whiteNoise.loop = true;
        whiteNoise.connect(filters[0]);
        whiteNoise.start();
        sourceNodeRef.current = whiteNoise;
      } else {
        // Pure tone 1000 Hz
        const osc = ctx.createOscillator();
        osc.frequency.value = 1000;
        osc.connect(filters[0]);
        osc.start();
        sourceNodeRef.current = osc;
      }

      setIsPlaying(true);
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <div className="flex flex-col min-h-screen">
      {/* Navigation Bar */}
      <nav className="sticky top-0 z-50 glass-panel border-b border-white/10 px-6 py-4 flex items-center justify-between">
        <div className="flex items-center space-x-3">
          <div className="w-10 h-10 rounded-xl bg-gradient-to-tr from-blue-600 to-cyan-400 flex items-center justify-center shadow-lg shadow-blue-500/30">
            <Headphones className="w-5 h-5 text-white" />
          </div>
          <div>
            <span className="text-xl font-bold tracking-tight bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-cyan-300 to-white">
              Amplify
            </span>
            <span className="ml-2 text-xs uppercase px-2 py-0.5 rounded-full bg-blue-500/10 text-blue-400 border border-blue-500/20 font-semibold">
              DSP v1.0
            </span>
          </div>
        </div>

        <div className="hidden md:flex items-center space-x-8 text-sm font-medium text-slate-300">
          <a href="#demo" className="hover:text-cyan-400 transition-colors">
            Interactive Demo
          </a>
          <a href="#architecture" className="hover:text-cyan-400 transition-colors">
            Architecture
          </a>
          <a href="#dsp" className="hover:text-cyan-400 transition-colors">
            DSP Formulas
          </a>
          <a href="#modules" className="hover:text-cyan-400 transition-colors">
            5 Modules
          </a>
        </div>

        <div className="flex items-center space-x-3">
          <a
            href="https://github.com/Rishi006knight/Sound-Amplification"
            target="_blank"
            rel="noreferrer"
            className="flex items-center space-x-2 px-4 py-2 rounded-lg glass-card hover:bg-white/10 transition text-sm font-medium"
          >
            <Github className="w-4 h-4" />
            <span>GitHub</span>
          </a>
          <a
            href="#demo"
            className="hidden sm:flex items-center space-x-2 px-4 py-2 rounded-lg bg-blue-600 hover:bg-blue-500 text-white font-medium text-sm transition shadow-lg shadow-blue-600/30"
          >
            <Sparkles className="w-4 h-4" />
            <span>Try Live DSP</span>
          </a>
        </div>
      </nav>

      {/* Hero Section */}
      <section className="relative px-6 pt-20 pb-16 max-w-6xl mx-auto text-center">
        <div className="inline-flex items-center space-x-2 px-3 py-1 rounded-full glass-card border border-cyan-500/30 text-cyan-400 text-xs font-semibold uppercase tracking-wider mb-6">
          <Zap className="w-3.5 h-3.5" />
          <span>Real-Time Windows System Audio Hearing Enhancement</span>
        </div>

        <h1 className="text-4xl sm:text-6xl font-extrabold tracking-tight text-white mb-6 leading-tight">
          Personalized Hearing Amplification <br className="hidden sm:inline" />
          <span className="bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-cyan-300 to-indigo-300">
            Engineered for Windows
          </span>
        </h1>

        <p className="text-lg sm:text-xl text-slate-400 max-w-3xl mx-auto mb-10 leading-relaxed">
          Amplify intercepts all Windows system audio (YouTube, Spotify, games, VLC) via WASAPI, passes it through an audiological multi-band DSP chain with NAL-R prescription targets, and outputs clear, comfortable sound to your headphones in real time.
        </p>

        <div className="flex flex-wrap items-center justify-center gap-4">
          <a
            href="#demo"
            className="px-6 py-3.5 rounded-xl bg-gradient-to-r from-blue-600 to-cyan-500 text-white font-semibold text-base shadow-xl shadow-blue-500/25 hover:shadow-blue-500/40 hover:scale-[1.02] transition-all flex items-center space-x-2"
          >
            <Activity className="w-5 h-5" />
            <span>Launch Web DSP Demo</span>
          </a>
          <a
            href="https://github.com/Rishi006knight/Sound-Amplification"
            target="_blank"
            rel="noreferrer"
            className="px-6 py-3.5 rounded-xl glass-card hover:bg-white/10 text-white font-semibold text-base border border-white/10 transition-all flex items-center space-x-2"
          >
            <Download className="w-5 h-5" />
            <span>Windows C++ Build</span>
          </a>
        </div>

        {/* Feature Badges */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 max-w-4xl mx-auto mt-16 text-left">
          <div className="p-4 rounded-xl glass-card border border-white/5">
            <div className="text-cyan-400 font-bold text-lg mb-1">&lt; 10 ms</div>
            <div className="text-slate-400 text-xs">WASAPI Near-Zero Latency</div>
          </div>
          <div className="p-4 rounded-xl glass-card border border-white/5">
            <div className="text-blue-400 font-bold text-lg mb-1">NAL-R &amp; NL2</div>
            <div className="text-slate-400 text-xs">Clinical Fitting Formulas</div>
          </div>
          <div className="p-4 rounded-xl glass-card border border-white/5">
            <div className="text-emerald-400 font-bold text-lg mb-1">MPO Protection</div>
            <div className="text-slate-400 text-xs">Brickwall Safety Limiter</div>
          </div>
          <div className="p-4 rounded-xl glass-card border border-white/5">
            <div className="text-amber-400 font-bold text-lg mb-1">A/B/C Audition</div>
            <div className="text-slate-400 text-xs">Hearing Loss Simulation</div>
          </div>
        </div>
      </section>

      {/* Interactive Web Audio Demo Section */}
      <section id="demo" className="px-6 py-16 max-w-6xl mx-auto">
        <div className="text-center mb-10">
          <h2 className="text-3xl font-bold text-white mb-3">Live Interactive Audiometry &amp; DSP Simulator</h2>
          <p className="text-slate-400 max-w-2xl mx-auto text-sm">
            Experience the real-time difference directly in your browser. Adjust hearing loss sliders, switch presets, and toggle between Normal sound, Simulated Hearing Loss, and Personalized Amplification.
          </p>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-12 gap-8 glass-panel p-6 sm:p-8 rounded-2xl border border-white/10 shadow-2xl">
          {/* Left Column: Preset & Audition Controls */}
          <div className="lg:col-span-4 flex flex-col space-y-6">
            {/* Presets Picker */}
            <div>
              <label className="block text-xs font-semibold uppercase tracking-wider text-slate-400 mb-2">
                1. Clinical Preset
              </label>
              <select
                value={selectedPreset}
                onChange={(e) => handlePresetChange(e.target.value)}
                className="w-full px-4 py-2.5 rounded-xl glass-input text-white text-sm focus:outline-none focus:border-cyan-500"
              >
                {PRESETS.map((p) => (
                  <option key={p.id} value={p.id} className="bg-slate-900 text-white">
                    {p.name}
                  </option>
                ))}
              </select>
              <p className="text-xs text-slate-400 mt-2 italic">
                {PRESETS.find((p) => p.id === selectedPreset)?.desc}
              </p>
            </div>

            {/* 3-Way Audition Mode Switcher */}
            <div>
              <label className="block text-xs font-semibold uppercase tracking-wider text-slate-400 mb-2">
                2. Auditioning Mode
              </label>
              <div className="grid grid-cols-3 gap-2">
                <button
                  onClick={() => setAuditionMode("bypass")}
                  className={`px-3 py-2 rounded-lg text-xs font-bold transition-all ${
                    auditionMode === "bypass"
                      ? "bg-slate-600 text-white shadow-lg"
                      : "glass-card text-slate-400 hover:text-white"
                  }`}
                >
                  A: Normal
                </button>
                <button
                  onClick={() => setAuditionMode("simulation")}
                  className={`px-3 py-2 rounded-lg text-xs font-bold transition-all ${
                    auditionMode === "simulation"
                      ? "bg-amber-600 text-white shadow-lg"
                      : "glass-card text-slate-400 hover:text-white"
                  }`}
                >
                  B: Loss Sim
                </button>
                <button
                  onClick={() => setAuditionMode("amplification")}
                  className={`px-3 py-2 rounded-lg text-xs font-bold transition-all ${
                    auditionMode === "amplification"
                      ? "bg-blue-600 text-white shadow-lg"
                      : "glass-card text-slate-400 hover:text-white"
                  }`}
                >
                  C: Amplified
                </button>
              </div>
            </div>

            {/* Prescription Formula */}
            <div>
              <label className="block text-xs font-semibold uppercase tracking-wider text-slate-400 mb-2">
                3. Fitting Formula
              </label>
              <div className="grid grid-cols-3 gap-2">
                <button
                  onClick={() => setFormula("nal_r")}
                  className={`px-3 py-1.5 rounded-lg text-xs font-medium ${
                    formula === "nal_r" ? "bg-cyan-600 text-white" : "glass-card text-slate-400"
                  }`}
                >
                  NAL-R
                </button>
                <button
                  onClick={() => setFormula("half_gain")}
                  className={`px-3 py-1.5 rounded-lg text-xs font-medium ${
                    formula === "half_gain" ? "bg-cyan-600 text-white" : "glass-card text-slate-400"
                  }`}
                >
                  1/2 Gain
                </button>
                <button
                  onClick={() => setFormula("pogo")}
                  className={`px-3 py-1.5 rounded-lg text-xs font-medium ${
                    formula === "pogo" ? "bg-cyan-600 text-white" : "glass-card text-slate-400"
                  }`}
                >
                  POGO
                </button>
              </div>
            </div>

            {/* Audio Player Controller */}
            <div className="p-4 rounded-xl bg-blue-950/40 border border-blue-500/20">
              <div className="flex items-center justify-between mb-3">
                <span className="text-xs font-semibold text-blue-300">Audio Test Stream</span>
                <select
                  value={soundType}
                  onChange={(e) => setSoundType(e.target.value as "speech" | "tone" | "pink")}
                  disabled={isPlaying}
                  className="px-2 py-1 text-xs rounded bg-slate-900 text-slate-200 border border-white/10"
                >
                  <option value="speech">Harmonic Speech Spectrum</option>
                  <option value="pink">Pink Noise</option>
                  <option value="tone">1000 Hz Pure Tone</option>
                </select>
              </div>

              <button
                onClick={toggleAudio}
                className={`w-full py-3 rounded-xl font-bold text-sm flex items-center justify-center space-x-2 transition shadow-lg ${
                  isPlaying
                    ? "bg-rose-600 hover:bg-rose-500 text-white"
                    : "bg-emerald-600 hover:bg-emerald-500 text-white"
                }`}
              >
                {isPlaying ? (
                  <>
                    <Square className="w-4 h-4 fill-white" />
                    <span>Stop Web Audio Engine</span>
                  </>
                ) : (
                  <>
                    <Play className="w-4 h-4 fill-white" />
                    <span>Start Live Audio Stream</span>
                  </>
                )}
              </button>
              <p className="text-[11px] text-slate-400 text-center mt-2">
                Use headphones for best frequency perception.
              </p>
            </div>
          </div>

          {/* Right Column: Interactive Audiogram Plot & Sliders */}
          <div className="lg:col-span-8 flex flex-col space-y-6">
            {/* SVG Audiogram Graph */}
            <div className="relative h-64 w-full glass-card rounded-xl p-4 flex flex-col">
              <div className="flex justify-between items-center mb-2 text-xs">
                <span className="font-semibold text-slate-300">Clinical Audiogram Curve (dB HL)</span>
                <div className="flex space-x-4">
                  <span className="text-blue-400 font-bold">● Left Ear (dB HL)</span>
                  <span className="text-red-400 font-bold">● Right Ear (dB HL)</span>
                </div>
              </div>

              <div className="relative flex-1 w-full bg-slate-950/60 rounded-lg p-2 overflow-hidden">
                {/* Normal Hearing Shaded Zone (0 to 20 dB HL) */}
                <div className="absolute left-10 right-2 top-2 h-[20%] bg-emerald-500/10 border-b border-emerald-500/20 pointer-events-none" />
                <span className="absolute right-4 top-3 text-[10px] text-emerald-400/80 font-medium pointer-events-none">
                  Normal Hearing Zone (&lt; 20 dB)
                </span>

                {/* SVG Curves */}
                <svg className="w-full h-full">
                  {/* Grid Lines */}
                  {[0, 20, 40, 60, 80, 100].map((db, idx) => {
                    const y = 10 + (idx / 5) * 80 + "%";
                    return (
                      <g key={db}>
                        <line x1="40" y1={y} x2="100%" y2={y} stroke="#334155" strokeWidth="1" />
                        <text x="5" y={y} fill="#64748b" fontSize="10" dominantBaseline="middle">
                          {db}
                        </text>
                      </g>
                    );
                  })}

                  {/* Left Ear Curve (Blue) */}
                  <polyline
                    fill="none"
                    stroke="#3b82f6"
                    strokeWidth="2.5"
                    points={leftLoss
                      .map((val, idx) => {
                        const x = 50 + (idx / 5) * (100 - 15) + "%";
                        const y = 10 + (val / 100) * 80 + "%";
                        return `${x},${y}`;
                      })
                      .join(" ")}
                  />

                  {/* Right Ear Curve (Red) */}
                  <polyline
                    fill="none"
                    stroke="#ef4444"
                    strokeWidth="2.5"
                    points={rightLoss
                      .map((val, idx) => {
                        const x = 50 + (idx / 5) * (100 - 15) + "%";
                        const y = 10 + (val / 100) * 80 + "%";
                        return `${x},${y}`;
                      })
                      .join(" ")}
                  />
                </svg>
              </div>
            </div>

            {/* 6 Frequency Sliders */}
            <div className="grid grid-cols-6 gap-2 sm:gap-4">
              {FREQUENCIES.map((freq, idx) => (
                <div key={freq} className="flex flex-col items-center glass-card p-2 rounded-xl">
                  <span className="text-xs font-bold text-slate-300 mb-1">{freq >= 1000 ? `${freq / 1000}k` : freq}Hz</span>
                  
                  {/* Vertical Slider */}
                  <input
                    type="range"
                    min="0"
                    max="100"
                    step="5"
                    value={leftLoss[idx]}
                    onChange={(e) => {
                      const val = parseInt(e.target.value);
                      const newL = [...leftLoss];
                      const newR = [...rightLoss];
                      newL[idx] = val;
                      newR[idx] = val;
                      setLeftLoss(newL);
                      setRightLoss(newR);
                    }}
                    className="w-full h-24 accent-blue-500 cursor-pointer -rotate-180"
                    style={{ writingMode: "vertical-lr" }}
                  />

                  <div className="mt-2 text-center">
                    <span className="text-xs font-semibold text-blue-400">{leftLoss[idx]} dB</span>
                    <div className="text-[10px] text-cyan-400 font-bold mt-0.5">
                      +{Math.round(calculateGain(leftLoss[idx], freq))} dB
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </div>
        </div>
      </section>

      {/* 5 Modules Breakdown Section */}
      <section id="modules" className="px-6 py-16 max-w-6xl mx-auto">
        <div className="text-center mb-12">
          <h2 className="text-3xl font-bold text-white mb-3">Modular System Architecture</h2>
          <p className="text-slate-400 max-w-2xl mx-auto text-sm">
            5 specialized modules spanning clinical assessment, prescription math, real-time C++ DSP, and Windows WASAPI audio.
          </p>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {/* Module 1 */}
          <div className="glass-panel p-6 rounded-2xl border border-blue-500/20 hover:border-blue-500/40 transition">
            <div className="w-10 h-10 rounded-xl bg-blue-600/20 text-blue-400 flex items-center justify-center mb-4">
              <BarChart3 className="w-5 h-5" />
            </div>
            <h3 className="text-lg font-bold text-white mb-2">Module 1: Audiometry &amp; Assessment</h3>
            <p className="text-slate-400 text-xs leading-relaxed mb-3">
              Collects pure-tone thresholds from 125 Hz to 8000 Hz, Air/Bone conduction, Most Comfortable Levels (MCL), and Uncomfortable Loudness Levels (UCL).
            </p>
            <span className="text-[11px] text-blue-400 font-semibold">Includes PTA3 &amp; Loss Categorization</span>
          </div>

          {/* Module 2 */}
          <div className="glass-panel p-6 rounded-2xl border border-cyan-500/20 hover:border-cyan-500/40 transition">
            <div className="w-10 h-10 rounded-xl bg-cyan-600/20 text-cyan-400 flex items-center justify-center mb-4">
              <Layers className="w-5 h-5" />
            </div>
            <h3 className="text-lg font-bold text-white mb-2">Module 2: Hearing Profile &amp; Fitting</h3>
            <p className="text-slate-400 text-xs leading-relaxed mb-3">
              Converts raw threshold matrices into frequency-dependent insertion gain curves using NAL-R, NAL-NL2, and Half-Gain mathematical prescriptions.
            </p>
            <span className="text-[11px] text-cyan-400 font-semibold">Separate Left &amp; Right Target Curves</span>
          </div>

          {/* Module 3 */}
          <div className="glass-panel p-6 rounded-2xl border border-indigo-500/20 hover:border-indigo-500/40 transition">
            <div className="w-10 h-10 rounded-xl bg-indigo-600/20 text-indigo-400 flex items-center justify-center mb-4">
              <Sliders className="w-5 h-5" />
            </div>
            <h3 className="text-lg font-bold text-white mb-2">Module 3: Personalized DSP Engine</h3>
            <p className="text-slate-400 text-xs leading-relaxed mb-3">
              8-band Linkwitz-Riley crossover filter bank, Wide Dynamic Range Compression (WDRC), and lookahead MPO brickwall safety limiting to prevent acoustic shock.
            </p>
            <span className="text-[11px] text-indigo-400 font-semibold">Zero-Overshoot MPO Protection</span>
          </div>

          {/* Module 4 */}
          <div className="glass-panel p-6 rounded-2xl border border-emerald-500/20 hover:border-emerald-500/40 transition">
            <div className="w-10 h-10 rounded-xl bg-emerald-600/20 text-emerald-400 flex items-center justify-center mb-4">
              <Volume2 className="w-5 h-5" />
            </div>
            <h3 className="text-lg font-bold text-white mb-2">Module 4: Windows WASAPI Audio</h3>
            <p className="text-slate-400 text-xs leading-relaxed mb-3">
              Captures all running Windows applications (YouTube, Spotify, VLC, Discord, Games) using WASAPI loopback and renders to headphones with &lt;10 ms latency.
            </p>
            <span className="text-[11px] text-emerald-400 font-semibold">Direct System-Wide Capture</span>
          </div>

          {/* Module 5 */}
          <div className="glass-panel p-6 rounded-2xl border border-amber-500/20 hover:border-amber-500/40 transition md:col-span-2 lg:col-span-1">
            <div className="w-10 h-10 rounded-xl bg-amber-600/20 text-amber-400 flex items-center justify-center mb-4">
              <Activity className="w-5 h-5" />
            </div>
            <h3 className="text-lg font-bold text-white mb-2">Module 5: Audiometry Simulation</h3>
            <p className="text-slate-400 text-xs leading-relaxed mb-3">
              Educational auditory recruitment simulator that allows clinicians, family members, and researchers to hear audio exactly as perceived with sensorineural loss.
            </p>
            <span className="text-[11px] text-amber-400 font-semibold">A/B/C Auditioning Mode</span>
          </div>
        </div>
      </section>

      {/* Footer */}
      <footer className="mt-auto border-t border-white/10 py-10 px-6 glass-panel">
        <div className="max-w-6xl mx-auto flex flex-col md:flex-row items-center justify-between text-xs text-slate-400 space-y-4 md:space-y-0">
          <div className="flex items-center space-x-2">
            <Headphones className="w-4 h-4 text-blue-400" />
            <span className="font-semibold text-slate-200">Amplify Hearing Assist Project</span>
            <span>— Open Source Audio Engineering</span>
          </div>
          <div>
            Built with C++20, JUCE, WASAPI, Next.js, and Web Audio API. Licensed under Apache-2.0.
          </div>
          <div className="flex space-x-6">
            <a href="https://github.com/Rishi006knight/Sound-Amplification" className="hover:text-white transition">
              GitHub Repository
            </a>
          </div>
        </div>
      </footer>
    </div>
  );
}
