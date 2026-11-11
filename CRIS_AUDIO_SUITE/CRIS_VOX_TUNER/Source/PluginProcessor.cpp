#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::StringArray makeKeys()
{
    return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
}

juce::AudioProcessorValueTreeState::ParameterLayout CrisVoxTunerAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<P>("correction", "Correction", 0.0f, 1.0f, 0.65f));
    p.push_back (std::make_unique<P>("speed", "Speed", 0.0f, 1.0f, 0.45f));
    p.push_back (std::make_unique<P>("humanize", "Humanize", 0.0f, 1.0f, 0.75f));
    p.push_back (std::make_unique<P>("mix", "Mix", 0.0f, 1.0f, 1.0f));
    p.push_back (std::make_unique<juce::AudioParameterChoice>("key", "Key", makeKeys(), 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice>("scale", "Scale", juce::StringArray{"Major","Minor"}, 1));
    return { p.begin(), p.end() };
}

CrisVoxTunerAudioProcessor::CrisVoxTunerAudioProcessor()
 : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                   .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
   parameters (*this, nullptr, "CRIS_TUNER_STATE", createLayout())
{
}

void CrisVoxTunerAudioProcessor::prepareToPlay (double sr, int block)
{
    pitchEngine.prepare (sr, getTotalNumInputChannels(), block);
}
void CrisVoxTunerAudioProcessor::releaseResources() { pitchEngine.reset(); }

bool CrisVoxTunerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void CrisVoxTunerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, buffer.getNumSamples());

    pitchEngine.process (buffer,
        parameters.getRawParameterValue("correction")->load(),
        parameters.getRawParameterValue("speed")->load(),
        parameters.getRawParameterValue("humanize")->load(),
        (int)parameters.getRawParameterValue("key")->load(),
        (int)parameters.getRawParameterValue("scale")->load(),
        parameters.getRawParameterValue("mix")->load());
}

juce::AudioProcessorEditor* CrisVoxTunerAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

void CrisVoxTunerAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}
void CrisVoxTunerAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CrisVoxTunerAudioProcessor();
}
