#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class CrisVoxEnhancerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit CrisVoxEnhancerAudioProcessorEditor(CrisVoxEnhancerAudioProcessor& p)
      : AudioProcessorEditor(&p), processor(p) { setSize(520,360); }
private:
    CrisVoxEnhancerAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrisVoxEnhancerAudioProcessorEditor)
};
