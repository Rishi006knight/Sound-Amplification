# Amplify Web Presentation & Interactive DSP Demo

This is the Next.js presentation and interactive Web Audio demonstration layer for the **Amplify** Windows Personalized Hearing Assistance project, designed for deployment on **Vercel**.

---

## 🚀 Features

- **Live Browser Web Audio DSP**: Real-time 6-band IIR filter bank running in the browser.
- **A/B/C Auditioning Switcher**: Toggle between Normal Audio, Simulated Hearing Loss, and Personalized Amplified Audio.
- **Interactive SVG Audiogram Graph**: Real-time visual curve updates matching 6 clinical octave sliders (250 Hz - 8000 Hz).
- **6 Clinical Presets**: 1-click loading for Presbycusis, Noise-Induced Notch, Cookie-Bite, and Flat Loss profiles.
- **Full Architecture Documentation**: Comprehensive breakdown of the 5 core engineering modules.

---

## 🛠 Local Development

```bash
# Navigate to web directory
cd web

# Install dependencies
npm install

# Start local Next.js dev server
npm run dev
```

Open [http://localhost:3000](http://localhost:3000) in your browser.

---

## 🌐 Deploying to Vercel

1. Push your repository to GitHub: `https://github.com/Rishi006knight/Sound-Amplification`
2. Log in to [Vercel](https://vercel.com).
3. Click **Add New Project** and import the repository.
4. Set the **Root Directory** to `web`.
5. Click **Deploy**.
