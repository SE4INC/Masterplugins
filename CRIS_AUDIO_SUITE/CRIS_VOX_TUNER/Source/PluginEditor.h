#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class CrisVoxTunerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit CrisVoxTunerAudioProcessorEditor (CrisVoxTunerAudioProcessor& p)
      : AudioProcessorEditor (&p), processor (p)
    {
        setSize (520, 320);
    }
private:
    CrisVoxTunerAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrisVoxTunerAudioProcessorEditor)
};
