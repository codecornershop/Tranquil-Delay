#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_processors_headless/juce_audio_processors_headless.h"
#include <JuceHeader.h>

//==============================================================================
/**
*/
class DelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DelayAudioProcessor();
    ~DelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameter Pointers
    juce::AudioParameterFloat* delayTimeParam;
    juce::AudioParameterFloat* feedbackParam;
    juce::AudioParameterFloat* dryWetParam;

    juce::AudioParameterBool* tempoSyncParam;
    juce::AudioParameterChoice* syncDivisionParam;
    juce::AudioParameterBool* getClearBufferParam() { return clearBufferParam; }

private:
    //==============================================================================
    // Delay line (circular buffer)
    juce::AudioBuffer<float> delayBuffer;
    int delayBufferSize;
    int writePosition;

    // Smoothed parameters (prevents clicks)
    juce::LinearSmoothedValue<float> smoothedDelayTime;
    juce::LinearSmoothedValue<float> smoothedFeedback;
    juce::LinearSmoothedValue<float> smoothedDryWet;

    //Sample rate cache
    double currentSampleRate;

    // Buffer clear on delay time change
    juce::AudioParameterBool* clearBufferParam;
    float lastDelayTime;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayAudioProcessor)
};
