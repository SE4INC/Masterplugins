#pragma once
#include <JuceHeader.h>

class PitchEngine
{
public:
    void prepare (double sampleRate, int channels, int maxBlockSize);
    void reset();
    void process (juce::AudioBuffer<float>& buffer,
                  float correctionAmount,
                  float speed,
                  float humanize,
                  int rootNote,
                  int scaleType,
                  float mix);

    float getDetectedMidiNote() const noexcept { return detectedMidi; }

private:
    float detectMidiNote (const juce::AudioBuffer<float>& buffer);
    float nearestAllowedMidi (float midi, int root, int scale) const;
    void processChannel (float* data, int numSamples, int channel, float ratio);

    double sr = 44100.0;
    int maxBlock = 512;
    int ringSize = 8192;
    std::vector<std::vector<float>> ring;
    std::vector<int> writePos;
    std::vector<float> phaseA, phaseB;
    float currentRatio = 1.0f;
    float detectedMidi = -1.0f;
    juce::AudioBuffer<float> dryBuffer;
    std::vector<float> analysisRing, analysisWindow;
    int analysisWrite = 0, analysisCount = 0;
};
