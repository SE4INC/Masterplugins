#pragma once
#include <JuceHeader.h>
#include "PitchEngine.h"

class CrisVoxTunerAudioProcessor : public juce::AudioProcessor
{
public:
    CrisVoxTunerAudioProcessor();
    ~CrisVoxTunerAudioProcessor() override = default;
    void prepareToPlay (double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "CRIS VOX TUNER"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    float getDetectedNote() const noexcept { return pitchEngine.getDetectedMidiNote(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    PitchEngine pitchEngine;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrisVoxTunerAudioProcessor)
};
