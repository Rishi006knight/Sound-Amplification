cmake_minimum_required(VERSION 3.22)

project(HearingAssist VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch JUCE via FetchContent if not found locally
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9 # or latest stable JUCE 7/8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Define JUCE Standalone Application / Plugin Target
juce_add_plugin(HearingAssist
    COMPANY_NAME "AmplifyAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_NAME "HearingAssist"
    PRODUCT_NAME "HearingAssist"
    FORMATS Standalone VST3
)

# Unified Production Source Files
target_sources(HearingAssist
    PRIVATE
        # Application Entry
        HearingAssist/App/Main.cpp

        # Audiometry
        HearingAssist/Audiometry/ThresholdData.h
        HearingAssist/Audiometry/ThresholdData.cpp

        # Prescription & Fitting
        HearingAssist/HearingProfile/FittingAlgorithms.h
        HearingAssist/HearingProfile/FittingAlgorithms.cpp

        # DSP Core
        HearingAssist/DSP/PersonalizedDSP.h
        HearingAssist/DSP/PersonalizedDSP.cpp
        HearingAssist/DSP/HearingAssistProcessor.h
        HearingAssist/DSP/HearingAssistProcessor.cpp

        # Storage & Persistence
        HearingAssist/Storage/ProfileStore.h
        HearingAssist/Storage/ProfileStore.cpp

        # UI & Visualization
        HearingAssist/UI/AudiogramComponent.h
        HearingAssist/UI/AudiogramComponent.cpp
        HearingAssist/UI/AudiogramView.h
        HearingAssist/UI/AudiogramView.cpp
)

target_include_directories(HearingAssist
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/HearingAssist
)

# Link essential JUCE modules (Audio, DSP, GUI)
target_link_libraries(HearingAssist
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Windows specific multimedia & system libraries
if(WIN32)
    target_link_libraries(HearingAssist
        PRIVATE
            ole32
            oleaut32
            uuid
            winmm
    )
endif()
