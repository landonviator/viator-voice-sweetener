#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ViatorvoicesweetenerAudioProcessor::ViatorvoicesweetenerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, _treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getButtonParams()[i]._id, this);
    }
    
    // init var states
    variableTree.setProperty("width", 0, nullptr);
    variableTree.setProperty("heigt", 0, nullptr);
    variableTree.setProperty("colorMenu", 1, nullptr);
    variableTree.setProperty("tooltipState", 1, nullptr);
}

ViatorvoicesweetenerAudioProcessor::~ViatorvoicesweetenerAudioProcessor()
{
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        _treeState.removeParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        _treeState.removeParameterListener(_parameterMap.getButtonParams()[i]._id, this);
    }
}

//==============================================================================
const juce::String ViatorvoicesweetenerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ViatorvoicesweetenerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ViatorvoicesweetenerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ViatorvoicesweetenerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ViatorvoicesweetenerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ViatorvoicesweetenerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ViatorvoicesweetenerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ViatorvoicesweetenerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ViatorvoicesweetenerAudioProcessor::getProgramName (int index)
{
    return {};
}

void ViatorvoicesweetenerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ViatorvoicesweetenerAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        auto param = _parameterMap.getSliderParams()[i];
        
        if (param.isInt == ViatorParameters::SliderParameterData::NumericType::kInt || param.isSkew == ViatorParameters::SliderParameterData::SkewType::kSkew)
        {
            auto range = juce::NormalisableRange<float>(param.min, param.max);
            
            if (param.isSkew == ViatorParameters::SliderParameterData::SkewType::kSkew)
            {
                range.setSkewForCentre(param.center);
            }
            
            params.push_back (std::make_unique<juce::AudioProcessorValueTreeState::Parameter>(juce::ParameterID { param.paramID, 1 }, param.name, param.name, range, param.initial, valueToTextFunction, textToValueFunction));
        }
        
        else
        {
            params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { param.paramID, 1 }, param.name, param.min, param.max, param.initial));
        }
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        auto param = _parameterMap.getButtonParams()[i];
        params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID { param._id, 1 }, param._name, false));
    }
        
    return { params.begin(), params.end() };
}

void ViatorvoicesweetenerAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)

{
    updateParameters();
}

void ViatorvoicesweetenerAudioProcessor::updateParameters()
{
    // params
    auto gain = _treeState.getRawParameterValue(ViatorParameters::inputID)->load();
    auto volume = _treeState.getRawParameterValue(ViatorParameters::outputID)->load();
    auto sweetener = _treeState.getRawParameterValue(ViatorParameters::sweetenerID)->load();
    
    // update
    _gainModule.setGainDecibels(gain);
    _volumeModule.setGainDecibels(volume);
    
    auto hpCutoffScaled = juce::jmap(sweetener, 0.0f, 100.0f, 20.0f, 40.0f);
    auto lowGainScaled = juce::jmap(sweetener, 0.0f, 100.0f, 0.0f, 6.0f);
    auto highGainScaled = juce::jmap(sweetener, 0.0f, 100.0f, 0.0f, 4.0f);
    _hpFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kCutoff, hpCutoffScaled);
    _lowFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kGain, lowGainScaled);
    _highFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kGain, highGainScaled);
}

//==============================================================================
void ViatorvoicesweetenerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _spec.sampleRate = sampleRate;
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    
    _hpFilter.prepare(_spec);
    _hpFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kType, viator_dsp::SVFilter<float>::kHighPass);
    _hpFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kQType, viator_dsp::SVFilter<float>::QType::kProportional);
    _hpFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kQ, 0.9f);
    _hpFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kGain, 0.0f);
    
    _lowFilter.prepare(_spec);
    _lowFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kType, viator_dsp::SVFilter<float>::kLowShelf);
    _lowFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kCutoff, 400.0f);
    _lowFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kQType, viator_dsp::SVFilter<float>::QType::kParametric);
    _lowFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kQ, 0.1f);
    
    _highFilter.prepare(_spec);
    _highFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kType, viator_dsp::SVFilter<float>::kHighShelf);
    _highFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kCutoff, 2000.0f);
    _highFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kQType, viator_dsp::SVFilter<float>::QType::kParametric);
    _highFilter.setParameter(viator_dsp::SVFilter<float>::ParameterId::kQ, 0.01f);
    
    _gainModule.prepare(_spec);
    _gainModule.setRampDurationSeconds(0.02);
    _volumeModule.prepare(_spec);
    _volumeModule.setRampDurationSeconds(0.02);
    
    updateParameters();
}

void ViatorvoicesweetenerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ViatorvoicesweetenerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void ViatorvoicesweetenerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::dsp::AudioBlock<float> block {buffer};
    
    // gain
    _gainModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    // filters
    _hpFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    _lowFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    _highFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    // volume
    _volumeModule.process(juce::dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool ViatorvoicesweetenerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ViatorvoicesweetenerAudioProcessor::createEditor()
{
    return new ViatorvoicesweetenerAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ViatorvoicesweetenerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    _treeState.state.appendChild(variableTree, nullptr);
    juce::MemoryOutputStream stream(destData, false);
    _treeState.state.writeToStream (stream);
}

void ViatorvoicesweetenerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, size_t(sizeInBytes));
    variableTree = tree.getChildWithName("Variables");
    
    if (tree.isValid())
    {
        _treeState.state = tree;
        _width = variableTree.getProperty("width");
        _height = variableTree.getProperty("height");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViatorvoicesweetenerAudioProcessor();
}
