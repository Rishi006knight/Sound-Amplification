/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    "./src/pages/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/components/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/app/**/*.{js,ts,jsx,tsx,mdx}",
  ],
  theme: {
    extend: {
      colors: {
        background: "#0d1117",
        surface: "#161b22",
        surfaceLight: "#21262d",
        primary: "#3b82f6",
        primaryHover: "#2563eb",
        accent: "#06b6d4",
        danger: "#ef4444",
        warning: "#f59e0b",
        success: "#10b981",
      },
    },
  },
  plugins: [],
};
