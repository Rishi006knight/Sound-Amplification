#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "DSP/HearingAssistProcessor.h"

class HearingAssistApplication : public juce::JUCEApplication {
public:
    HearingAssistApplication() = default;

    const juce::String getApplicationName() override { return "HearingAssist"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& /*commandLineParameters*/) override {
        // Initialize Standalone audio device manager and processor window
        mainWindow = std::make_unique<MainWindow>(getApplicationName(), new HearingAssistProcessor());
    }

    void shutdown() override {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override {}

    // Main application window holding the standalone processor and editor
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(const juce::String& name, HearingAssistProcessor* processor)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel()
                                 .findColour(juce::ResizableWindow::backgroundColourId),
                             DocumentWindow::allButtons),
              processorHolder(processor) {
            setUsingNativeTitleBar(true);

            // Create default standalone audio device selector & processor editor
            if (auto* editor = processorHolder->createEditorIfNeeded()) {
                setContentOwned(editor, true);
            } else {
                setContentOwned(new juce::GenericAudioProcessorEditor(*processorHolder), true);
            }

            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        std::unique_ptr<HearingAssistProcessor> processorHolder;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// Application entry point macro
START_JUCE_APPLICATION(HearingAssistApplication)
