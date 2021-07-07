/*
 ==============================================================================
 
 This file was auto-generated!
 
 It contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "ESStandalone.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioProcessorValueTreeState::ParameterLayout ESAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    String n;
    //==============================================================================
    // Top level parameters
    
    n = "Master";
    layout.add (std::make_unique<AudioParameterFloat> (n, n, 0., 2., 1.));
    paramIds.add(n);
    
    for (int i = 0; i < NUM_MACROS; ++i)
    {
        n = "M" + String(i+1);
        layout.add (std::make_unique<AudioParameterFloat> (n, n, 0., 1., 0.));
        paramIds.add(n);
    }
    
    for (int i = 0; i < NUM_CHANNELS; ++i)
    {
        n = "PitchBend" + String(i);
        layout.add (std::make_unique<AudioParameterFloat> (n, n, -24., 24., 0.));
        paramIds.add(n);
    }
    
    //==============================================================================
    for (int i = 0; i < NUM_OSCS; ++i)
    {
        String n = "Osc" + String(i+1);
        layout.add (std::make_unique<AudioParameterChoice> (n, n, StringArray("Off", "On"), 1));
        paramIds.add(n);
        
        for (int j = 0; j < cOscParams.size(); ++j)
        {
            float min = vOscInit[j][0];
            float max = vOscInit[j][1];
            float def = vOscInit[j][2];
            
            n = "Osc" + String(i+1) + " " + cOscParams[j];
            layout.add (std::make_unique<AudioParameterFloat> (n, n, min, max, def));
            paramIds.add(n);
        }
        
        n = "Osc" + String(i+1) + " ShapeSet";
        layout.add (std::make_unique<AudioParameterChoice> (n, n, oscShapeSetNames, 0));
        paramIds.add(n);
        
        n = "Osc" + String(i+1) + " FilterSend";
        layout.add (std::make_unique<AudioParameterFloat> (n, n, 0.f, 1.f, 0.5f));
        paramIds.add(n);
    }
    
    //==============================================================================
    
    for (int i = 0; i < NUM_FILT; ++i)
    {
        n = "Filter" + String(i+1);
        layout.add (std::make_unique<AudioParameterChoice> (n, n, StringArray("Off", "On"), 1));
        paramIds.add(n);
        
        n = "Filter" + String(i+1) + " Type";
        layout.add (std::make_unique<AudioParameterChoice> (n, n, oscShapeSetNames, 0));
        paramIds.add(n);
        
        for (int j = 0; j < cFilterParams.size(); ++j)
        {
            float min = vFilterInit[j][0];
            float max = vFilterInit[j][1];
            float def = vFilterInit[j][2];
            
            n = "Filter" + String(i+1) + " " + cFilterParams[j];
            layout.add (std::make_unique<AudioParameterFloat> (n, n, min, max, def));
            paramIds.add(n);
        }
    }
    
    n = "Filter Series-Parallel Mix";
    layout.add (std::make_unique<AudioParameterFloat> (n, n, 0., 1., 0.));
    paramIds.add(n);
    
    //=============================================================================
    for (int i = 0; i < NUM_ENVS; ++i)
    {
        for (int j = 0; j < cEnvelopeParams.size(); ++j)
        {
            float min = vEnvelopeInit[j][0];
            float max = vEnvelopeInit[j][1];
            float def = vEnvelopeInit[j][2];
            
            n = "Envelope" + String(i+1) + " " + cEnvelopeParams[j];
            layout.add (std::make_unique<AudioParameterFloat> (n, n, min, max, def));
            paramIds.add(n);
        }
        
        n = "Envelope" + String(i+1) + " Velocity";
        layout.add (std::make_unique<AudioParameterChoice> (n, n, StringArray("Off", "On"), 1));
        paramIds.add(n);
    }
    
    //==============================================================================
    for (int i = 0; i < NUM_LFOS; ++i)
    {
        for (int j = 0; j < cLowFreqParams.size(); ++j)
        {
            float min = vLowFreqInit[j][0];
            float max = vLowFreqInit[j][1];
            float def = vLowFreqInit[j][2];
            
            n = "LFO" + String(i+1) + " " + cLowFreqParams[j];
            layout.add (std::make_unique<AudioParameterFloat> (n, n, min, max, def));
            paramIds.add(n);
        }
        
        n = "LFO" + String(i+1) + " ShapeSet";
        layout.add (std::make_unique<AudioParameterChoice> (n, n, oscShapeSetNames, 0));
        paramIds.add(n);
        
        n = "LFO" + String(i+1) + " Sync";
        layout.add (std::make_unique<AudioParameterChoice> (n, n, StringArray("Off", "On"), 0));
        paramIds.add(n);
    }
    
    //==============================================================================
    for (int i = 0; i < cOutputParams.size(); ++i)
    {
        float min = vOutputInit[i][0];
        float max = vOutputInit[i][1];
        float def = vOutputInit[i][2];
        n = "Output " + cOutputParams[i];
        layout.add (std::make_unique<AudioParameterFloat> (n, n, min, max, def));
        paramIds.add(n);
    }
    
    //==============================================================================
    for (int i = 1; i < CopedentColumnNil; ++i)
    {
        n = cCopedentColumnNames[i];
        layout.add (std::make_unique<AudioParameterChoice>(n, n, StringArray("Off", "On"), 0));
    }
    
    DBG("PARAMS//");
    for (int i = 0; i < paramIds.size(); ++i)
    {
        DBG(paramIds[i] + ": " + String(i));
    }
    
    return layout;
}

//==============================================================================
//==============================================================================
ESAudioProcessor::ESAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  )
#endif
,
vts(*this, nullptr, juce::Identifier ("Parameters"), createParameterLayout())
{
    formatManager.registerBasicFormats();   
    keyboardState.addListener(this);
    
    LEAF_init(&leaf, 44100.0f, dummy_memory, 1, []() {return (float)rand() / RAND_MAX; });
    
    leaf.clearOnAllocation = 1;
    
    tSimplePoly_init(&strings[0], 12, &leaf);
    tSimplePoly_setNumVoices(&strings[0], 1);
    
    voiceNote[0] = 0;
    for (int i = 1; i < NUM_STRINGS; ++i)
    {
        tSimplePoly_init(&strings[i], 1, &leaf);
        voiceNote[i] = 0;
    }
    
    for (int i = 0; i < 12; ++i)
    {
        centsDeviation[i] = 0.f;
    }

    leaf.clearOnAllocation = 0;
    
    for (int i = 0; i < NUM_OSCS; ++i)
    {
        oscs.add(new Oscillator("Osc" + String(i+1), *this, vts));
    }
    
    for (int i = 0; i < NUM_FILT; ++i)
    {
        filt.add(new Filter("Filter" + String(i+1), *this, vts));
    }
    
    seriesParallel = std::make_unique<SmoothedParameter>(*this, vts, "Filter Series-Parallel Mix", -1);
    
    for (int i = 0; i < NUM_MACROS; ++i)
    {
        String n = "M" + String(i+1);
        ccParams.add(new SmoothedParameter(*this, vts, n, -1));
        ccSources.add(new MappingSourceModel(*this, n, ccParams.getLast()->getValuePointerArray(),
                                             false, false, false, Colours::red));
        addMappingSource(ccSources.getLast());
        sourceIds.add(n);
    }
    
    for (int i = 0; i < NUM_ENVS; ++i)
    {
        String n = "Envelope" + String(i+1);
        envs.add(new Envelope(n, *this, vts));
        addMappingSource(envs.getLast());
        sourceIds.add(n);
    }
    
    for (int i = 0; i < NUM_LFOS; ++i)
    {
        String n = "LFO" + String(i+1);
        lfos.add(new LowFreqOscillator(n, *this, vts));
        addMappingSource(lfos.getLast());
        sourceIds.add(n);
    }
    
    output = std::make_unique<Output>("Output", *this, vts);
    
    for (int i = 0; i < NUM_CHANNELS; ++i)
    {
        pitchBendParams.add(new SmoothedParameter(*this, vts, "PitchBend" + String(i), i-1));
        midiChannelNoteCount[i+1] = 0;
        midiChannelActivity[i+1] = 0;
        channelToString[i+1] = i;
    }
    
    //==============================================================================
    
    for (int i = 0; i < CopedentColumnNil; ++i)
    {
        copedentArray.add(Array<float>());
        for (int v = 0; v < NUM_STRINGS; ++v)
        {
            copedentArray.getReference(i).add(cCopedentArrayInit[i][v]);
        }
    }
    copedentFundamental = 21.f;
    
    // A couple of default mappings that will be used if nothing has been saved
    Mapping defaultFilter1Cutoff;
    defaultFilter1Cutoff.sourceName = "Envelope3";
    defaultFilter1Cutoff.targetName = "Filter1 Cutoff T3";
    defaultFilter1Cutoff.value = 24.f;
    
    Mapping defaultOutputAmp;
    defaultOutputAmp.sourceName = "Envelope4";
    defaultOutputAmp.targetName = "Output Amp T3";
    defaultOutputAmp.value = 1.f;
    
    initialMappings.add(defaultFilter1Cutoff);
    initialMappings.add(defaultOutputAmp);
    
    DBG("SOURCES//");
    for (int i = 0; i < sourceIds.size(); ++i)
    {
        DBG(sourceIds[i] + ": " + String(i));
    }
}

ESAudioProcessor::~ESAudioProcessor()
{
    params.clearQuick(false);
}

//==============================================================================
void ESAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    midiChannelActivityTimeout = sampleRate/samplesPerBlock/2;
    LEAF_setSampleRate(&leaf, sampleRate);
    
    for (auto env : envs)
    {
        env->prepareToPlay(sampleRate, samplesPerBlock);
    }
    for (auto lfo : lfos)
    {
        lfo->prepareToPlay(sampleRate, samplesPerBlock);
    }
    for (auto osc : oscs)
    {
        osc->prepareToPlay(sampleRate, samplesPerBlock);
    }
    for (auto f : filt)
    {
        f->prepareToPlay(sampleRate, samplesPerBlock);
    }
    output->prepareToPlay(sampleRate, samplesPerBlock);
    
    for (auto param : params)
    {
        param->prepareToPlay(sampleRate, samplesPerBlock);
    }
    
    if (initialMappings.isEmpty())
    {
        // Source addresses have likely changed, so remap existing mappings
        for (auto target : targetMap)
        {
            if (target->currentSource != nullptr)
            {
                target->setMapping(target->currentSource, target->value, false);
            }
        }
    }
    else // First prepareToPlay
    {
        for (Mapping m : initialMappings)
        {
            targetMap[m.targetName]->setMapping(sourceMap[m.sourceName], m.value, false);
        }
        initialMappings.clear();
    }
}

void ESAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ESAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

union uintfUnion
{
    float f;
    uint32_t i;
};

void ESAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear (i, 0, buffer.getNumSamples());
    
    MidiMessage m;
    for (MidiMessageMetadata metadata : midiMessages) {
        m = metadata.getMessage();
        handleMidiMessage(m);
    }
    
    if (waitingToSendPreset)
    {
        Array<float> data;
        
        // Parameter values
        // Order is determined in createParameterLayout
        for (auto id : paramIds)
        {
            const NormalisableRange<float>& range = vts.getParameter(id)->getNormalisableRange();
            data.add(range.convertFrom0to1(vts.getParameter(id)->getValue()));
        }
        
        // Mappings
        for (auto id : paramIds)
        {
            for (int t = 0; t < 3; ++t)
            {
                String tn = id + " T" + String(t+1);
                if (targetMap.contains(tn))
                {
                    MappingTargetModel* target = targetMap[tn];
                    if (MappingSourceModel* source = target->currentSource)
                    {
                        data.add(sourceIds.indexOf(source->name));//SourceID
                        data.add(paramIds.indexOf(target->name));//TargetID
                        data.add(t);//TargetIndex
                        data.add(target->value);//Value
                    }
                }
            }
        }
        
        Array<uint8_t> data7bitInt;
        union uintfUnion fu;
        
        for (int i = 0; i < data.size(); i++)
        {
            data7bitInt.add(0); // saying it's a preset
            
            fu.f = data[i];
            data7bitInt.add((fu.i >> 28) & 15);
            data7bitInt.add((fu.i >> 21) & 127);
            data7bitInt.add((fu.i >> 14) & 127);
            data7bitInt.add((fu.i >> 7) & 127);
            data7bitInt.add(fu.i & 127);
            
            MidiMessage presetMessage = MidiMessage::createSysExMessage(data7bitInt.getRawDataPointer(), sizeof(uint8_t) * data7bitInt.size());
            
            midiMessages.addEvent(presetMessage, 0);
        }
        waitingToSendPreset = false;
    }
    
    if (waitingToSendCopedent)
    {
        Array<float> flat;
    
        for (int i = 0; i < copedentArray.size(); ++i)
        {
            for (auto value : copedentArray.getReference(i))
            {
                flat.add(value);
            }
        }

        //RangedAudioParameter* fund = vts.getParameter("Copedent Fundamental");
        //flat.add(fund->convertFrom0to1(fund->getValue()));
        
        Array<uint8_t> flat7bitInt;
        union uintfUnion fu;
        
        for (int j = 0; j < 11; j++)
        {
            flat7bitInt.clear();
            
            flat7bitInt.add(1); // saying it's a copedent
            flat7bitInt.add(copedentNumber); // saying which copedent number to store (need this to be a user entered value)
            flat7bitInt.add(50 + j);
            
            for (int i = 0; i < 12; i++)
            {
                fu.f = flat[i + (j*12)];
                flat7bitInt.add((fu.i >> 28) & 15);
                flat7bitInt.add((fu.i >> 21) & 127);
                flat7bitInt.add((fu.i >> 14) & 127);
                flat7bitInt.add((fu.i >> 7) & 127);
                flat7bitInt.add(fu.i & 127);
            }
            
            MidiMessage copedentMessage = MidiMessage::createSysExMessage(flat7bitInt.getRawDataPointer(), sizeof(uint8_t) * flat7bitInt.size());
            
            midiMessages.addEvent(copedentMessage, 0);
        }
        waitingToSendCopedent = false;
    }
    
    Array<float> resolvedCopedent;
    for (int r = 0; r < NUM_STRINGS; ++r)
    {
        float minBelowZero = 0.0f;
        float maxAboveZero = 0.0f;
        for (int c = 1; c < CopedentColumnNil; ++c)
        {
            if (vts.getParameter(cCopedentColumnNames[c])->getValue() > 0)
            {
                float value = copedentArray.getReference(c)[r];
                if (value < minBelowZero) minBelowZero = value;
                else if (value > maxAboveZero) maxAboveZero = value;
            }
        }
        resolvedCopedent.add(minBelowZero + maxAboveZero);
    }
    
    for (int i = 0; i < envs.size(); ++i)
    {
        envs[i]->frame();
    }
    for (int i = 0; i < lfos.size(); ++i)
    {
        lfos[i]->frame();
    }
    for (int i = 0; i < oscs.size(); ++i)
    {
        oscs[i]->frame();
    }
    for (int i = 0; i < filt.size(); ++i)
    {
        filt[i]->frame();
    }
    output->frame();

    float parallel = seriesParallel->tickNoHooks();
    
    int mpe = mpeMode ? 1 : 0;
    int impe = 1-mpe;
    
    for (int s = 0; s < buffer.getNumSamples(); s++)
    {
        for (int i = 0; i < ccParams.size(); ++i)
        {
            ccParams[i]->tickNoHooks();
        }
        
        float globalPitchBend = pitchBendParams[0]->tick(s);
        
        float samples[2][NUM_STRINGS];
        float outputSamples[2];
        outputSamples[0] = 0.f;
        outputSamples[1] = 0.f;
        
        for (int v = 0; v < NUM_STRINGS; ++v)
        {
            float pitchBend = globalPitchBend + pitchBendParams[v+1]->tickNoHooks();
            float tempNote = (float)tSimplePoly_getPitch(&strings[v*mpe], v*impe);
            tempNote += resolvedCopedent[v];
            tempNote += pitchBend;
            float tempPitchClass = ((((int)tempNote) - keyCenter) % 12 );
            float tunedNote = tempNote + centsDeviation[(int)tempPitchClass];
            voiceNote[v] = tunedNote;
            samples[0][v] = 0.f;
            samples[1][v] = 0.f;
        }
        
        for (int i = 0; i < envs.size(); ++i)
        {
            envs[i]->tick();
        }
        for (int i = 0; i < lfos.size(); ++i)
        {
            lfos[i]->tick();
        }
        for (int i = 0; i < oscs.size(); ++i)
        {
            oscs[i]->tick(samples);
        }

        filt[0]->tick(samples[0]);
        
        for (int v = 0; v < NUM_STRINGS; ++v)
        {
            samples[1][v] += samples[0][v]*(1.f-parallel);
        }
        
        filt[1]->tick(samples[1]);
        
        for (int v = 0; v < NUM_STRINGS; ++v)
        {
            samples[1][v] += samples[0][v]*parallel;
        }
        
        output->tick(samples[1], outputSamples, totalNumOutputChannels);
    
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            buffer.setSample(channel, s, outputSamples[channel]);
        }
    }
    
    for (int i = 0; i < NUM_CHANNELS; ++i)
        if (midiChannelActivity[i] > 0) midiChannelActivity[i]--;
}

//==============================================================================
bool ESAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ESAudioProcessor::createEditor()
{
    return new ESAudioProcessorEditor (*this, vts);
}

//==============================================================================
void ESAudioProcessor::handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    if (mpeMode) midiChannelNoteCount[midiChannel]++;
    noteOn(midiChannel, midiNoteNumber, velocity);
}

void ESAudioProcessor::handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    if (mpeMode) midiChannelNoteCount[midiChannel]--;
    noteOff(midiChannel, midiNoteNumber, velocity);
}

//==============================================================================
void ESAudioProcessor::handleMidiMessage(const MidiMessage& m)
{
    if (m.isNoteOnOrOff())
    {
        keyboardState.processNextMidiEvent(m);
    }
    else
    {
        int channel = mpeMode ? m.getChannel() : 1;
        if (mpeMode) midiChannelActivity[channel] = midiChannelActivityTimeout;
        if (m.isPitchWheel())
        {
            pitchBend(channel, m.getPitchWheelValue());
        }
        else if (m.isController())
        {
            ctrlInput(channel, m.getControllerNumber(), m.getControllerValue());
        }
    }
}

void ESAudioProcessor::noteOn(int channel, int key, float velocity)
{
    int i = mpeMode ? channelToString[channel]-1 : 0;
    if (i < 0) return;
    if (!velocity) noteOff(channel, key, velocity);
    else
    {
        int v = tSimplePoly_noteOn(&strings[i], key, velocity * 127.f);
        if (!mpeMode) i = v;
        
        if (v >= 0)
        {
            for (auto e : envs) e->noteOn(i, velocity);
            for (auto o : lfos) o->noteOn(i, velocity);
        }
    }
}

void ESAudioProcessor::noteOff(int channel, int key, float velocity)
{
    int i = mpeMode ? channelToString[channel]-1 : 0;
    if (i < 0) return;
    // if we're monophonic, we need to allow fast voice stealing and returning
    // to previous stolen notes without regard for the release envelopes
    int v = tSimplePoly_noteOff(&strings[i], key);
    if (!mpeMode) i = v;
    
    if (v >= 0)
    {
        for (auto e : envs) e->noteOff(i, velocity);
        for (auto o : lfos) o->noteOff(i, velocity);
    }
}

void ESAudioProcessor::pitchBend(int channel, int data)
{
    // Parameters need to be set with a 0. to 1. range, but will use their set range when accessed
    float bend = data / 16383.f;
    if (mpeMode)
    {
        vts.getParameter("PitchBend" + String(channelToString[channel]))->setValueNotifyingHost(bend);
    }
    else
    {
        vts.getParameter("PitchBend0")->setValueNotifyingHost(bend);
    }
}

void ESAudioProcessor::ctrlInput(int channel, int ctrl, int value)
{
    float v = value / 127.f;
    if (channel == 1)
    {
        if (1 <= ctrl && ctrl <= NUM_MACROS)
        {
            vts.getParameter("M" + String(ctrl))->setValueNotifyingHost(v);
        }
    }
}

void ESAudioProcessor::sustainOff()
{
    
}

void ESAudioProcessor::sustainOn()
{
    
}

void ESAudioProcessor::toggleBypass()
{
    
}

void ESAudioProcessor::toggleSustain()
{
    
}

//==============================================================================
bool ESAudioProcessor::midiChannelIsActive(int channel)
{
    return midiChannelNoteCount[channel] + midiChannelActivity[channel] > 0;
}

//==============================================================================
bool ESAudioProcessor::getMPEMode()
{
    return mpeMode;
}

void ESAudioProcessor::setMPEMode(bool enabled)
{
    mpeMode = enabled;
    tSimplePoly_setNumVoices(&strings[0], mpeMode ? 1 : 12);
}

//==============================================================================
void ESAudioProcessor::sendCopedentMidiMessage()
{
    waitingToSendCopedent = true;
}

void ESAudioProcessor::sendPresetMidiMessage()
{
    waitingToSendPreset = true;
}

//==============================================================================
const juce::String ESAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ESAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ESAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ESAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ESAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ESAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int ESAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ESAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ESAudioProcessor::getProgramName (int index)
{
    return {};
}

void ESAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ESAudioProcessor::addMappingSource(MappingSourceModel* source)
{
    sourceMap.set(source->name, source);
}

void ESAudioProcessor::addMappingTarget(MappingTargetModel* target)
{
    targetMap.set(target->name, target);
}

MappingSourceModel* ESAudioProcessor::getMappingSource(const String& name)
{
    return sourceMap.getReference(name);
}

MappingTargetModel* ESAudioProcessor::getMappingTarget(const String& name)
{
    return targetMap.getReference(name);
}

//==============================================================================
void ESAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    ValueTree root ("Electrosteel");
    
    // Top level settings
    root.setProperty("editorScale", editorScale, nullptr);
    root.setProperty("mpeMode", mpeMode, nullptr);
    for (int i = 0; i < NUM_CHANNELS; ++i)
    {
        root.setProperty("Ch" + String(i+1) + "String", channelToString[i+1], nullptr);
    }
    
    for (int i = 0; i < NUM_OSCS; ++i)
    {
        root.setProperty("osc" + String(i+1) + "File",
                         oscs[i]->getWaveTableFile().getFullPathName(), nullptr);
    }
    for (int i = 0; i < NUM_LFOS; ++i)
    {
        root.setProperty("lfo" + String(i+1) + "File",
                         lfos[i]->getWaveTableFile().getFullPathName(), nullptr);
    }
    
    // Audio processor value tree state
    ValueTree state = vts.copyState();
    root.addChild(state, -1, nullptr);
    
    // Copedent
    ValueTree copedent ("Copedent");
    copedent.setProperty("number", copedentNumber, nullptr);
    copedent.setProperty("name", copedentName, nullptr);
    copedent.setProperty("fundamental", copedentFundamental, nullptr);
    for (int c = 0; c < copedentArray.size(); ++c)
    {
        ValueTree column ("c" + String(c));
        for (int r = 0; r < copedentArray.getReference(c).size(); ++r)
        {
            column.setProperty("r" + String(r), copedentArray.getReference(c)[r], nullptr);
        }
        copedent.addChild(column, -1, nullptr);
    }
    root.addChild(copedent, -1, nullptr);
    
    // Mappings
    ValueTree mappings ("Mappings");
    int i = 0;
    for (auto target : targetMap)
    {
        if (MappingSourceModel* source = target->currentSource)
        {
            ValueTree mapping ("m" + String(i++));
            mapping.setProperty("s", source->name, nullptr);
            mapping.setProperty("t", target->name, nullptr);
            mapping.setProperty("v", target->value, nullptr);
            mappings.addChild(mapping, -1, nullptr);
        }
    }
    root.addChild(mappings, -1, nullptr);
    
    std::unique_ptr<XmlElement> xml = root.createXml();
    copyXmlToBinary (*xml, destData);
}

void ESAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    
    if (xml.get() != nullptr)
    {
        // Top level settings
        editorScale = xml->getDoubleAttribute("editorScale", 1.05);
        setMPEMode(xml->getBoolAttribute("mpeMode", true));
        
        for (int i = 0; i < NUM_CHANNELS; ++i)
        {
            channelToString[i+1] = xml->getIntAttribute("Ch" + String(i+1) + "String", i);
        }
        
        for (int i = 0; i < NUM_OSCS; ++i)
        {
            File wav (xml->getStringAttribute("osc" + String(i+1) + "File"));
            if (wav.exists())
            {
                waveTableFiles.addIfNotAlreadyThere(wav);
                oscs[i]->setWaveTableFile(wav);
                oscs[i]->setLoadingTables(true);
                oscs[i]->clearWaveTables();
                oscs[i]->addWaveTables(wav);
                oscs[i]->waveTablesChanged();
            }
        }
        for (int i = 0; i < NUM_LFOS; ++i)
        {
            File wav (xml->getStringAttribute("lfo" + String(i+1) + "File"));
            if (wav.exists())
            {
                waveTableFiles.addIfNotAlreadyThere(wav);
                lfos[i]->setWaveTableFile(wav);
                lfos[i]->setLoadingTables(true);
                lfos[i]->clearWaveTables();
                lfos[i]->addWaveTables(wav);
                lfos[i]->waveTablesChanged();
            }
        }
        
        // Audio processor value tree state
        if (XmlElement* state = xml->getChildByName(vts.state.getType()))
            vts.replaceState (juce::ValueTree::fromXml (*state));
        
        // Copedent
        if (XmlElement* copedent = xml->getChildByName("Copedent"))
        {
            copedentNumber = copedent->getIntAttribute("number");
            copedentName = copedent->getStringAttribute("name");
            copedentFundamental = copedent->getDoubleAttribute("fundamental");
            for (int c = 0; c < copedentArray.size(); ++c)
            {
                XmlElement* column = copedent->getChildByName("c" + String(c));
                for (int r = 0; r < copedentArray.getReference(c).size(); ++r)
                {
                    float value = column->getDoubleAttribute("r" + String(r));
                    copedentArray.getReference(c).set(r, value);
                }
            }
        }
        
        // Mappings
        if (XmlElement* mappings = xml->getChildByName("Mappings"))
        {
            initialMappings.clear();
            for (auto child : mappings->getChildIterator())
            {
                Mapping m;
                m.sourceName = child->getStringAttribute("s");
                m.targetName = child->getStringAttribute("t");
                m.value = child->getDoubleAttribute("v");
                initialMappings.add(m);
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ESAudioProcessor();
}

//PARAMS//
//Master: 0
//M1: 1
//M2: 2
//M3: 3
//M4: 4
//M5: 5
//M6: 6
//M7: 7
//M8: 8
//M9: 9
//M10: 10
//M11: 11
//M12: 12
//M13: 13
//M14: 14
//M15: 15
//M16: 16
//PitchBend0: 17
//PitchBend1: 18
//PitchBend2: 19
//PitchBend3: 20
//PitchBend4: 21
//PitchBend5: 22
//PitchBend6: 23
//PitchBend7: 24
//PitchBend8: 25
//PitchBend9: 26
//PitchBend10: 27
//PitchBend11: 28
//PitchBend12: 29
//Osc1: 30
//Osc1 Pitch: 31
//Osc1 Fine: 32
//Osc1 Shape: 33
//Osc1 Amp: 34
//Osc1 ShapeSet: 35
//Osc1 FilterSend: 36
//Osc2: 37
//Osc2 Pitch: 38
//Osc2 Fine: 39
//Osc2 Shape: 40
//Osc2 Amp: 41
//Osc2 ShapeSet: 42
//Osc2 FilterSend: 43
//Osc3: 44
//Osc3 Pitch: 45
//Osc3 Fine: 46
//Osc3 Shape: 47
//Osc3 Amp: 48
//Osc3 ShapeSet: 49
//Osc3 FilterSend: 50
//Filter1: 51
//Filter1 Type: 52
//Filter1 Cutoff: 53
//Filter1 Resonance: 54
//Filter1 KeyFollow: 55
//Filter2: 56
//Filter2 Type: 57
//Filter2 Cutoff: 58
//Filter2 Resonance: 59
//Filter2 KeyFollow: 60
//Filter Series-Parallel Mix: 61
//Envelope1 Attack: 62
//Envelope1 Decay: 63
//Envelope1 Sustain: 64
//Envelope1 Release: 65
//Envelope1 Leak: 66
//Envelope1 Velocity: 67
//Envelope2 Attack: 68
//Envelope2 Decay: 69
//Envelope2 Sustain: 70
//Envelope2 Release: 71
//Envelope2 Leak: 72
//Envelope2 Velocity: 73
//Envelope3 Attack: 74
//Envelope3 Decay: 75
//Envelope3 Sustain: 76
//Envelope3 Release: 77
//Envelope3 Leak: 78
//Envelope3 Velocity: 79
//Envelope4 Attack: 80
//Envelope4 Decay: 81
//Envelope4 Sustain: 82
//Envelope4 Release: 83
//Envelope4 Leak: 84
//Envelope4 Velocity: 85
//LFO1 Rate: 86
//LFO1 Shape: 87
//LFO1 Sync Phase: 88
//LFO1 ShapeSet: 89
//LFO1 Sync: 90
//LFO2 Rate: 91
//LFO2 Shape: 92
//LFO2 Sync Phase: 93
//LFO2 ShapeSet: 94
//LFO2 Sync: 95
//LFO3 Rate: 96
//LFO3 Shape: 97
//LFO3 Sync Phase: 98
//LFO3 ShapeSet: 99
//LFO3 Sync: 100
//LFO4 Rate: 101
//LFO4 Shape: 102
//LFO4 Sync Phase: 103
//LFO4 ShapeSet: 104
//LFO4 Sync: 105
//Output Amp: 106
//Output Pan: 107

//SOURCES//
//M1: 0
//M2: 1
//M3: 2
//M4: 3
//M5: 4
//M6: 5
//M7: 6
//M8: 7
//M9: 8
//M10: 9
//M11: 10
//M12: 11
//M13: 12
//M14: 13
//M15: 14
//M16: 15
//Envelope1: 16
//Envelope2: 17
//Envelope3: 18
//Envelope4: 19
//LFO1: 20
//LFO2: 21
//LFO3: 22
//LFO4: 23

//Mapping looks like:
//SourceID, TargetParamID, TargetIndex(0-2), Scale
