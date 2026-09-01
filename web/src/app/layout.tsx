import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Amplify | Personalized Hearing Amplification & Simulation Engine",
  description:
    "Real-time Windows system audio personalized hearing assistance software with clinical audiogram compensation, NAL-R fitting, and hearing loss simulation.",
  keywords: [
    "hearing assistance",
    "audiogram",
    "NAL-R",
    "DSP",
    "JUCE",
    "WASAPI loopback",
    "hearing loss simulation",
    "audio processing",
  ],
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en" className="dark scroll-smooth">
      <body className="min-h-screen bg-[#090d16] text-slate-100 antialiased selection:bg-blue-600 selection:text-white">
        {children}
      </body>
    </html>
  );
}
