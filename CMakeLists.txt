cmake_minimum_required(VERSION 3.20)
project(Amplify
    VERSION 1.0.0
    DESCRIPTION "Windows-based Personalized Hearing Amplification & Simulation Software"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings and optimizations
if(MSVC)
    add_compile_options(/W4 /EHsc /O2 /fp:fast)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
else()
    add_compile_options(-Wall -Wextra -O3 -ffast-math)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Core & DSP Source Files
set(AMPLIFY_CORE_SOURCES
    src/Core/AudiogramData.cpp
    src/Core/HearingProfile.cpp
    src/Core/Presets.cpp
)

set(AMPLIFY_FITTING_SOURCES
    src/Fitting/NALRFormula.cpp
    src/Fitting/HalfGainFormula.cpp
    src/Fitting/NALNL2Estimator.cpp
    src/Fitting/FittingEngine.cpp
)

set(AMPLIFY_DSP_SOURCES
    src/DSP/FilterBank.cpp
    src/DSP/BandCompressor.cpp
    src/DSP/SafetyLimiter.cpp
    src/DSP/HearingLossSimulator.cpp
    src/DSP/MasterDSPChain.cpp
)

set(AMPLIFY_AUDIO_SOURCES
    src/Audio/WASAPILoopback.cpp
    src/Audio/WASAPIRenderer.cpp
    src/Audio/AudioRouter.cpp
    src/Audio/WavePlayer.cpp
)

set(AMPLIFY_STORAGE_SOURCES
    src/Storage/ProfileSerializer.cpp
)

set(AMPLIFY_UI_SOURCES
    src/UI/AudiogramView.cpp
    src/UI/FittingView.cpp
    src/UI/DSPControlView.cpp
    src/UI/SimulationView.cpp
    src/UI/AudioControlView.cpp
    src/UI/SpectrumVisualizer.cpp
    src/UI/DashboardView.cpp
)

# Library target containing all core, DSP, fitting, audio, storage, and UI logic
add_library(AmplifyCore STATIC
    ${AMPLIFY_CORE_SOURCES}
    ${AMPLIFY_FITTING_SOURCES}
    ${AMPLIFY_DSP_SOURCES}
    ${AMPLIFY_AUDIO_SOURCES}
    ${AMPLIFY_STORAGE_SOURCES}
    ${AMPLIFY_UI_SOURCES}
)

target_include_directories(AmplifyCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Windows Multimedia and WASAPI libraries
if(WIN32)
    target_link_libraries(AmplifyCore PUBLIC
        ole32
        oleaut32
        uuid
        mmdevapi
        avrt
        winmm
    )
endif()

# Main Application Executable
add_executable(AmplifyApp
    src/Main.cpp
)

target_link_libraries(AmplifyApp PRIVATE AmplifyCore)

# Test Suite Executable
add_executable(AmplifyTests
    tests/TestMain.cpp
    tests/TestFittingFormulas.cpp
    tests/TestDSPChain.cpp
    tests/TestProfileSerializer.cpp
)

target_link_libraries(AmplifyTests PRIVATE AmplifyCore)

enable_testing()
add_test(NAME AllAmplifyTests COMMAND AmplifyTests)
