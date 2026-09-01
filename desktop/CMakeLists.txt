cmake_minimum_required(VERSION 3.22)

project(PersonalHearingAssistant VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch JUCE automatically via CMake FetchContent
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Define Standalone Application and VST3 Targets
juce_add_plugin(PersonalHearingAssistant
    COMPANY_NAME "AmplifyAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_NAME "PersonalHearingAssistant"
    PRODUCT_NAME "PersonalHearingAssistant"
    FORMATS Standalone VST3
)

# Desktop Source Files
target_sources(PersonalHearingAssistant
    PRIVATE
        # Application Launcher
        Source/App/Main.cpp

        # Audiometry Subsystem (Module 1)
        Source/Audiometry/ThresholdData.h
        Source/Audiometry/ThresholdData.cpp

        # Hearing Profile & Fitting Algorithms (Module 2)
        Source/HearingProfile/FittingAlgorithms.h
        Source/HearingProfile/FittingAlgorithms.cpp

        # DSP Core Pipeline & Audio Processor (Module 3 & 4)
        Source/DSP/PersonalizedDSP.h
        Source/DSP/PersonalizedDSP.cpp
        Source/DSP/HearingAssistProcessor.h
        Source/DSP/HearingAssistProcessor.cpp

        # Profile Persistence
        Source/Storage/ProfileStore.h
        Source/Storage/ProfileStore.cpp

        # GUI Components (Module 1, 2, 5 UI)
        Source/UI/AudiogramComponent.h
        Source/UI/AudiogramComponent.cpp
        Source/UI/AudiogramView.h
        Source/UI/AudiogramView.cpp
)

target_include_directories(PersonalHearingAssistant
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Link essential JUCE modules
target_link_libraries(PersonalHearingAssistant
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
    target_link_libraries(PersonalHearingAssistant
        PRIVATE
            ole32
            oleaut32
            uuid
            winmm
    )
endif()
