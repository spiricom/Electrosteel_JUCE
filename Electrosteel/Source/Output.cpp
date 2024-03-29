/*
  ==============================================================================

    Output.cpp
    Created: 17 Jun 2021 10:57:13pm
    Author:  Matthew Wang

  ==============================================================================
*/

#include "Output.h"
#include "PluginProcessor.h"

//==============================================================================
Output::Output(const String& n, ESAudioProcessor& p,
               AudioProcessorValueTreeState& vts) :
AudioComponent(n, p, vts, cOutputParams, false)
{
    master = std::make_unique<SmoothedParameter>(processor, vts, "Master");
    
    int temp = processor.leaf.clearOnAllocation;
    processor.leaf.clearOnAllocation = 1;
    tOversampler_init(&os[0], MASTER_OVERSAMPLE, 0, &processor.leaf);
    tOversampler_init(&os[1], MASTER_OVERSAMPLE, 0, &processor.leaf);
    processor.leaf.clearOnAllocation = temp;
}

Output::~Output()
{
    tOversampler_free(&os[0]);
    tOversampler_free(&os[1]);
}

void Output::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    AudioComponent::prepareToPlay(sampleRate, samplesPerBlock);
}

void Output::frame()
{
    sampleInBlock = 0;
}

void Output::tick(float input[NUM_STRINGS], float output[2], int numChannels)
{
//    float a = sampleInBlock * invBlockSize;
    float m = master->tickNoHooksNoSmoothing();
    
    for (int v = 0; v < processor.numVoicesActive; ++v)
    {
        float amp = quickParams[OutputAmp][v]->tick();
        float pan = quickParams[OutputPan][v]->tick();
        amp = amp < 0.f ? 0.f : amp;
        pan = LEAF_clip(-1.f, pan, 1.f);
        
        float sample = input[v] * amp;
        
        // Porting over some code from
        // https://github.com/juce-framework/JUCE/blob/master/modules/juce_dsp/processors/juce_Panner.cpp
        
        if (numChannels > 1)
        {
            float normPan = 0.5f * (pan+1.f);
            
            // balanced
            //        float lg = jmin(0.5f, 1.f - normPan);
            //        float rg = jmin(0.5f, normPan);
            //        float boost = 2.f;
            
            // linear
            //        float lg = 1.f - normPan;
            //        float rg = normPan;
            //        float boost = 2.f;
            
            // sin3dB
            float lg = std::sinf(0.5f * PI * (1.f - normPan));
            float rg = std::sinf(0.5f * PI * normPan);
            float boost = LEAF_SQRT2;
            
            // sin6dB
            //        float lg = std::powf(std::sin(0.5f * PI * (1.f - normPan)), 2.f);
            //        float rg = std::powf(std::sinf(0.5f * PI * normPan), 2.f);
            //        float boost = 2.f;
            
            // squareRoot3dB
            //        float lg = std::sqrtf(1.f - normPan);
            //        float rg = std::sqrtf(normPan);
            //        float boost = LEAF_SQRT2;
            
            output[0] += sample*lg*boost;
            output[1] += sample*rg*boost;
        }
        else output[0] += sample;
    }
    
    float pedGain = 1.f;
    if (processor.pedalControlsMaster)
    {
        // this is to clip the gain settings so all the way down on the pedal isn't actually
        // off, it let's a little signal through. Would be more efficient to fix the table to
        // span a better range.
        float volumeSmoothed = processor.ccParams.getLast()->get();
        float volIdx = LEAF_clip(47.0f, ((volumeSmoothed * 80.0f) + 47.0f), 127.0f);
        
        //then interpolate the value
        int volIdxInt = (int) volIdx;
        float alpha = volIdx-volIdxInt;
        int volIdxIntPlus = (volIdxInt + 1) & 127;
        float omAlpha = 1.0f - alpha;
        pedGain = volumeAmps128[volIdxInt] * omAlpha;
        pedGain += volumeAmps128[volIdxIntPlus] * alpha;
    }
    
    //JS - I added a final saturator - would sound a little better in the plugin with oversampling, too. Could just oversample the distortion by 4 and see how that feels.
    output[0] = tOversampler_tick(&os[0], output[0], oversamplerArray, &tanhf);
    output[1] = tOversampler_tick(&os[1], output[1], oversamplerArray, &tanhf);
    output[0] = output[0] * m * pedGain;
    output[1] = output[1] * m * pedGain;
    
    sampleInBlock++;
}
