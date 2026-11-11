#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout CrisVoxEnhancerAudioProcessor::createLayout()
{
    using F = juce::AudioParameterFloat;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<F>("lowcut","Low Cut",20.0f,250.0f,75.0f));
    p.push_back(std::make_unique<F>("mid","Mid Body", -6.0f, 6.0f, 0.0f));
    p.push_back(std::make_unique<F>("presence","Presence", -6.0f, 9.0f, 2.0f));
    p.push_back(std::make_unique<F>("air","Air", -6.0f, 9.0f, 1.5f));
    p.push_back(std::make_unique<F>("warmth","Warmth",0.0f,1.0f,0.15f));
    p.push_back(std::make_unique<F>("deess","De-esser",0.0f,1.0f,0.2f));
    p.push_back(std::make_unique<F>("mix","Mix",0.0f,1.0f,1.0f));
    p.push_back(std::make_unique<F>("output","Output",-18.0f,6.0f,0.0f));
    return {p.begin(),p.end()};
}

CrisVoxEnhancerAudioProcessor::CrisVoxEnhancerAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
   parameters(*this,nullptr,"CRIS_ENHANCER_STATE",createLayout())
{}

void CrisVoxEnhancerAudioProcessor::prepareToPlay(double sr,int block)
{
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
    lowCut.prepare(spec); midBody.prepare(spec); presence.prepare(spec); air.prepare(spec); deEss.prepare(spec);
    dry.setSize(getTotalNumOutputChannels(),block);
    lowCut.reset(); midBody.reset(); presence.reset(); air.reset(); deEss.reset();
}
void CrisVoxEnhancerAudioProcessor::releaseResources(){}
bool CrisVoxEnhancerAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    auto in=l.getMainInputChannelSet(), out=l.getMainOutputChannelSet();
    return in==out && (in==juce::AudioChannelSet::mono() || in==juce::AudioChannelSet::stereo());
}
void CrisVoxEnhancerAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n=b.getNumSamples(), ch=b.getNumChannels();
    if(dry.getNumSamples()<n || dry.getNumChannels()<ch) dry.setSize(ch,n,false,false,true);
    for(int c=0;c<ch;++c) dry.copyFrom(c,0,b,c,0,n);

    const float lc=parameters.getRawParameterValue("lowcut")->load();
    const float mid=parameters.getRawParameterValue("mid")->load();
    const float pr=parameters.getRawParameterValue("presence")->load();
    const float ar=parameters.getRawParameterValue("air")->load();
    const float warmth=parameters.getRawParameterValue("warmth")->load();
    const float deess=parameters.getRawParameterValue("deess")->load();
    const float mix=parameters.getRawParameterValue("mix")->load();
    const float outDb=parameters.getRawParameterValue("output")->load();

    *lowCut.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(),lc);
    *midBody.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),900.0f,0.8f,juce::Decibels::decibelsToGain(mid));
    *presence.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),3500.0f,0.8f,juce::Decibels::decibelsToGain(pr));
    *air.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(),11000.0f,0.7f,juce::Decibels::decibelsToGain(ar));
    *deEss.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(),8500.0f,0.7f,juce::Decibels::decibelsToGain(-6.0f * deess));

    juce::dsp::AudioBlock<float> block(b);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    lowCut.process(ctx); midBody.process(ctx); presence.process(ctx); air.process(ctx); deEss.process(ctx);

    const float drive=1.0f + warmth*3.0f;
    const float gain=juce::Decibels::decibelsToGain(outDb);
    for(int c=0;c<ch;++c)
    {
        auto* x=b.getWritePointer(c);
        for(int i=0;i<n;++i)
        {
            float y=std::tanh(x[i]*drive)/std::tanh(drive);
            x[i]=((1.0f-mix)*dry.getSample(c,i)+mix*y)*gain;
        }
    }
}
juce::AudioProcessorEditor* CrisVoxEnhancerAudioProcessor::createEditor(){return new juce::GenericAudioProcessorEditor(*this);}
void CrisVoxEnhancerAudioProcessor::getStateInformation(juce::MemoryBlock& d)
{
    auto s=parameters.copyState(); std::unique_ptr<juce::XmlElement> x(s.createXml()); copyXmlToBinary(*x,d);
}
void CrisVoxEnhancerAudioProcessor::setStateInformation(const void* d,int sz)
{
    std::unique_ptr<juce::XmlElement> x(getXmlFromBinary(d,sz));
    if(x && x->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*x));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new CrisVoxEnhancerAudioProcessor();}
