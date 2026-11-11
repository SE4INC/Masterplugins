#include "PitchEngine.h"
#include <cmath>

void PitchEngine::prepare (double sampleRate, int channels, int maxBlockSize)
{
    sr = sampleRate;
    maxBlock = juce::jmax (maxBlockSize, 64);
    ringSize = 8192;
    ring.assign ((size_t) channels, std::vector<float> ((size_t) ringSize, 0.0f));
    writePos.assign ((size_t) channels, 0);
    phaseA.assign ((size_t) channels, 0.0f);
    phaseB.assign ((size_t) channels, 0.5f);
    dryBuffer.setSize (channels, maxBlock);
    analysisRing.assign (4096, 0.0f);
    analysisWindow.assign (2048, 0.0f);
    analysisWrite = analysisCount = 0;
    reset();
}

void PitchEngine::reset()
{
    for (auto& ch : ring) std::fill (ch.begin(), ch.end(), 0.0f);
    std::fill (writePos.begin(), writePos.end(), 0);
    std::fill (phaseA.begin(), phaseA.end(), 0.0f);
    std::fill (phaseB.begin(), phaseB.end(), 0.5f);
    currentRatio = 1.0f;
    detectedMidi = -1.0f;
    std::fill (analysisRing.begin(), analysisRing.end(), 0.0f);
    analysisWrite = analysisCount = 0;
}

float PitchEngine::detectMidiNote (const juce::AudioBuffer<float>& buffer)
{
    // Rolling analysis window allows detection of lower baritone notes even with  block sizes of 256/512.
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float mono = 0.0f;
        for (int c = 0; c < buffer.getNumChannels(); ++c)
            mono += buffer.getReadPointer(c)[i] / (float) buffer.getNumChannels();
        analysisRing[(size_t)analysisWrite] = mono;
        analysisWrite = (analysisWrite + 1) % (int)analysisRing.size();
        analysisCount = juce::jmin (analysisCount + 1, (int)analysisRing.size());
    }

    const int n = (int)analysisWindow.size();
    if (analysisCount < n) return -1.0f;
    for (int i = 0; i < n; ++i)
        analysisWindow[(size_t)i] = analysisRing[(size_t)((analysisWrite + (int)analysisRing.size() - n + i) % (int)analysisRing.size())];

    float rms = 0.0f;
    for (float x : analysisWindow) rms += x * x;
    rms = std::sqrt (rms / (float)n);
    if (rms < 0.003f) return -1.0f;

    const int minLag = juce::jmax (2, (int)(sr / 700.0));
    const int maxLag = juce::jmin (n / 2, (int)(sr / 65.0));
    float best = 0.0f;
    int bestLag = -1;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double corr = 0.0, e1 = 0.0, e2 = 0.0;
        for (int i = 0; i < n - lag; i += 2)
        {
            const float a = analysisWindow[(size_t)i], b = analysisWindow[(size_t)(i + lag)];
            corr += (double)a * b; e1 += (double)a * a; e2 += (double)b * b;
        }
        const float norm = (e1 > 1.0e-12 && e2 > 1.0e-12)
                         ? (float)(corr / std::sqrt(e1 * e2)) : 0.0f;
        if (norm > best) { best = norm; bestLag = lag; }
    }
    if (bestLag <= 0 || best < 0.55f) return -1.0f;
    const float hz = (float)sr / (float)bestLag;
    if (hz < 65.0f || hz > 700.0f) return -1.0f;
    return 69.0f + 12.0f * std::log2 (hz / 440.0f);
}

float PitchEngine::nearestAllowedMidi (float midi, int root, int scale) const
{
    static const int major[] = { 0, 2, 4, 5, 7, 9, 11 };
    static const int minor[] = { 0, 2, 3, 5, 7, 8, 10 };
    const int* intervals = (scale == 1 ? minor : major);
    float bestNote = std::round (midi);
    float bestDistance = 1000.0f;
    const int centre = (int)std::floor (midi);
    for (int note = centre - 12; note <= centre + 12; ++note)
    {
        int pc = ((note - root) % 12 + 12) % 12;
        bool allowed = false;
        for (int i = 0; i < 7; ++i) if (intervals[i] == pc) allowed = true;
        if (allowed && std::abs (midi - (float)note) < bestDistance)
        {
            bestDistance = std::abs (midi - (float)note);
            bestNote = (float)note;
        }
    }
    return bestNote;
}

void PitchEngine::processChannel (float* data, int numSamples, int channel, float ratio)
{
    auto& rb = ring[(size_t)channel];
    const int size = ringSize;
    const float range = 1800.0f; // ~41 ms at 44.1 kHz
    const float minDelay = 300.0f;
    const float phaseInc = (1.0f - ratio) / range;
    const bool unity = std::abs (ratio - 1.0f) < 0.002f;

    auto readAt = [&] (float delay)
    {
        float pos = (float)writePos[(size_t)channel] - delay;
        while (pos < 0.0f) pos += (float)size;
        while (pos >= (float)size) pos -= (float)size;
        const int i0 = (int)pos;
        const int i1 = (i0 + 1) % size;
        const float frac = pos - (float)i0;
        return rb[(size_t)i0] + frac * (rb[(size_t)i1] - rb[(size_t)i0]);
    };

    for (int i = 0; i < numSamples; ++i)
    {
        const float input = data[i];
        rb[(size_t)writePos[(size_t)channel]] = input;
        const float pa = phaseA[(size_t)channel];
        const float pb = phaseB[(size_t)channel];
        const float da = minDelay + pa * range;
        const float db = minDelay + pb * range;
        const float wa = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * pa);
        const float wb = 1.0f - wa;
        const float shifted = readAt (da) * wa + readAt (db) * wb;
        // Unity-ratio bypass avoids unnecessary delay when no correction is required.
        data[i] = unity ? input : shifted;

        phaseA[(size_t)channel] += phaseInc;
        phaseB[(size_t)channel] += phaseInc;
        if (phaseA[(size_t)channel] >= 1.0f) phaseA[(size_t)channel] -= 1.0f;
        if (phaseA[(size_t)channel] < 0.0f) phaseA[(size_t)channel] += 1.0f;
        if (phaseB[(size_t)channel] >= 1.0f) phaseB[(size_t)channel] -= 1.0f;
        if (phaseB[(size_t)channel] < 0.0f) phaseB[(size_t)channel] += 1.0f;
        writePos[(size_t)channel] = (writePos[(size_t)channel] + 1) % size;
    }
}

void PitchEngine::process (juce::AudioBuffer<float>& buffer, float correctionAmount,
                           float speed, float humanize, int rootNote, int scaleType, float mix)
{
    const int channels = juce::jmin (buffer.getNumChannels(), (int)ring.size());
    const int samples = buffer.getNumSamples();
    if (channels == 0) return;

    if (dryBuffer.getNumSamples() < samples || dryBuffer.getNumChannels() < channels)
        dryBuffer.setSize (channels, samples, false, false, true);
    for (int c = 0; c < channels; ++c)
        dryBuffer.copyFrom (c, 0, buffer, c, 0, samples);

    const float detected = detectMidiNote (buffer);
    if (detected >= 0.0f)
    {
        detectedMidi = detected;
        const float target = nearestAllowedMidi (detected, rootNote, scaleType);
        const float semitones = (target - detected) * juce::jlimit (0.0f, 1.0f, correctionAmount);
        const float targetRatio = std::pow (2.0f, semitones / 12.0f);
        const float smoothing = juce::jlimit (0.001f, 0.25f, (1.0f - speed) * 0.12f + humanize * 0.02f);
        currentRatio += (targetRatio - currentRatio) * smoothing;
    }

    // Avoid extreme shifts in this first-generation real-time engine.
    currentRatio = juce::jlimit (0.90f, 1.12f, currentRatio);
    for (int c = 0; c < channels; ++c)
    {
        processChannel (buffer.getWritePointer(c), samples, c, currentRatio);
        buffer.applyGain (c, 0, samples, juce::jlimit (0.0f, 1.0f, mix));
        buffer.addFrom (c, 0, dryBuffer, c, 0, samples,
                        1.0f - juce::jlimit (0.0f, 1.0f, mix));
    }
}
