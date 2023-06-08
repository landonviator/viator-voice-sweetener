#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class NavBar;
class PresetBrowser  : public juce::Component, public juce::ComboBox::Listener
{
public:
    PresetBrowser(ViatorvoicesweetenerAudioProcessor&);
    ~PresetBrowser() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void updateMenuWithPresets();
    
    enum class Direction
    {
        kLeft,
        kRight
    };

private:
    
    ViatorvoicesweetenerAudioProcessor& audioProcessor;
    
    // buttons
    juce::OwnedArray<viator_gui::TextButton> _buttons;
    juce::StringArray _buttonNameTexts =
    {
      "<", ">"
    };
    
    viator_gui::Menu _presetMenu;
    juce::Array<juce::File> _xmlFiles;
    juce::StringArray _xmlFilePaths;
    
private:
    
    // buttons
    void initButtons();
    int getButtonIndex(const juce::String& name);
    void navigateMenuOnButtonClick(Direction newDirection);
    
    // menu
    void initMenu();
    void attachMenu();
    
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override;
    
private:
    
    const int _numButtons = 2;
    bool isSettingSelectionProgrammatically = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBrowser)
};
