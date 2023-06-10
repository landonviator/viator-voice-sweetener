#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "includes.h"
#include "LAF/Colors.h"

class ViatorvoicesweetenerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::ChangeListener
{
public:
    ViatorvoicesweetenerAudioProcessorEditor (ViatorvoicesweetenerAudioProcessor&);
    ~ViatorvoicesweetenerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    ViatorThemes::ViatorThemeData& getThemeData()
    {
            return _theme;
    }
    
    void setTooltipText(const juce::String& tooltip);

private:
    ViatorvoicesweetenerAudioProcessor& audioProcessor;
    
    // theme
    ViatorThemes::ViatorThemeData _theme;
    
    // comps
    Header _headerComp;
    SettingsPage _settingsComp;
    
    // dials
    juce::OwnedArray<viator_gui::Dial> _pluginDials;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> _pluginSliderAttachments;
    juce::OwnedArray<viator_gui::Dial> _ioDials;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> _ioSliderAttachments;
    
    // tooltip
    viator_gui::Label _tooltipLabel;
    juce::StringArray _pluginDialTooltips =
    {
      "Increases the sweetness of your voical quality."
    };
    
    juce::StringArray _ioDialTooltips =
    {
      "Sets the volume of the signal coming into the plugin.", "Sets the volume of the signal coming out of the plugin."
    };
    
private:
    // window
    void setWindowSizeLogic();
    
    // background
    void setBackgroundStyle(juce::Graphics& g);
    
    // settings
    void setSettingsState(bool isActive);
    void initOverlayProps();
    
    // change listener
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    
    // dials
    void updateSliderColors();
    void initDialProps();
    
    // tooltip
    void initTooltipLabel();
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    
private:
    const int _numPluginDials = 1;
    const int _numIODials = 2;
    juce::Rectangle<int> _headerArea;
    juce::Rectangle<int> _ioArea;
    juce::Rectangle<int> _pluginArea;
    
    std::vector<juce::String> _pluginDialNames =
    {
      "Sweeten"
    };
    
    std::vector<juce::String> _ioDialNames =
    {
        "Input", "Master Volume"
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViatorvoicesweetenerAudioProcessorEditor)
};
