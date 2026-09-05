#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "juce_audio_processors_headless/juce_audio_processors_headless.h"
#include "juce_core/juce_core.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      delayBufferSize(0),
      writePosition(0),
      currentSampleRate(44100.0)
{
  // Create Parameters
  delayTimeParam = new juce::AudioParameterFloat (
      "delayTime",
      "Delay Time",
      juce::NormalisableRange<float>(0.01f, 2.0f, 0.001f, 0.3f), // range (seconds)
      1.0f
      );

  feedbackParam = new juce::AudioParameterFloat (
      "feedback",
      "Feedback",
      juce::NormalisableRange<float>(0.0f, 0.95f, 0.001f),
      0.3f
      );

  dryWetParam = new juce::AudioParameterFloat (
      "dryWet",
      "Dry/Wet",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
      0.5f
      );

  tempoSyncParam = new juce::AudioParameterBool ("tempoSync", "Tempo Sync", false);

  syncDivisionParam = new juce::AudioParameterChoice (
      "syncDivision",
      "Sync Division",
      juce::StringArray (
        "1/64",
        "1/64.",
        "1/32",
        "1/32.",
        "1/16", 
        "1/16.",
        "1/8", 
        "1/8.", 
        "1/4", 
        "1/4.", 
        "1/2",
        "1/2."
        ),
      0
  );

  clearBufferParam = new juce::AudioParameterBool (
      "clearBuffer",
      "Clear Buffer on Time Change",
      false
  );

  lastDelayTime = 0.5f;

  addParameter(delayTimeParam);
  addParameter(feedbackParam);
  addParameter(dryWetParam);

  addParameter(tempoSyncParam);
  addParameter(syncDivisionParam);
  addParameter(clearBufferParam);

  // Initialize smoothed values
  smoothedDelayTime.reset(currentSampleRate, 0.02);
  smoothedFeedback.reset(currentSampleRate, 0.02);
  smoothedDryWet.reset(currentSampleRate, 0.02);
}

DelayAudioProcessor::~DelayAudioProcessor()
{
}
//==============================================================================
void DelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  currentSampleRate = sampleRate;

  // Allocated delay buffer for 2 seconds of stereo audio
  int maxDelaySamples = (int)(sampleRate * 2.0);
  delayBufferSize = maxDelaySamples * 2;
  delayBuffer.setSize(2, delayBufferSize);
  delayBuffer.clear();

  writePosition = 0;

  // Reset smoothed values
  smoothedDelayTime.reset(sampleRate, 0.0001);
  smoothedFeedback.reset(sampleRate, 0.02);
  smoothedDryWet.reset(sampleRate, 0.02);

  // Set initial values
  smoothedDelayTime.setTargetValue(*delayTimeParam);
  smoothedFeedback.setTargetValue(*feedbackParam);
  smoothedDryWet.setTargetValue(*dryWetParam);
}

void DelayAudioProcessor::releaseResources()
{
  // Nothing to release
}

//==============================================================================
void DelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
  juce::ScopedNoDenormals noDenormals;

  // Get the current smoothed values
  float currentDelayTime = *delayTimeParam;

  if (*tempoSyncParam) {
    double bpm = 120.0;
    if (auto* playHead = getPlayHead()) {
      if (auto position = playHead->getPosition()) {
        if (auto bpmOpt = position->getBpm()) {
          bpm = *bpmOpt;
        }
      }
    }

    int divisionIndex = (int)*syncDivisionParam;
    // Calculate delay time in seconds based on tempo and division
    double divisor = 1.0;
    switch (divisionIndex) {
      case 0: divisor = 0.03125; break;
      case 1: divisor = 0.046875; break;
      case 2: divisor = 0.0625; break;
      case 3: divisor = 0.09375; break;
      case 4: divisor = 0.125; break;
      case 5: divisor = 0.1875; break;
      case 6: divisor = 0.25; break;
      case 7: divisor = 0.375; break;
      case 8: divisor = 0.5; break;
      case 9: divisor = 0.75; break;
      case 10: divisor = 1.0; break;
      case 11: divisor = 1.5; break;
    }
    double syncDelayTime = (60.0 / bpm) * divisor;
    // use syncDelayTime instead of the knob value
    currentDelayTime = (float)syncDelayTime;
  }

  // Update smoothed parameters (prevents clicks)
  smoothedDelayTime.setTargetValue(currentDelayTime);
  float smoothedDelay = smoothedDelayTime.getNextValue();

  
  float rawDelayTime = *delayTimeParam;
  if (rawDelayTime != lastDelayTime && *clearBufferParam) {
    delayBuffer.clear();
    writePosition = 0;
    lastDelayTime = rawDelayTime;
  }

  smoothedFeedback.setTargetValue(*feedbackParam);
  smoothedDryWet.setTargetValue(*dryWetParam);
  float currentFeedback = smoothedFeedback.getNextValue();
  float currentDryWet = smoothedDryWet.getNextValue();

  // Calculate delay in samples
  int delaySamples = (int)(smoothedDelay * currentSampleRate);
  if (delaySamples < 1) delaySamples = 1;

  // Get channel pointer
  auto* leftInput = buffer.getReadPointer(0);
  auto* rightInput = buffer.getReadPointer(1);
  auto* leftOutput = buffer.getWritePointer(0);
  auto* rightOutput = buffer.getWritePointer(1);
  int numSamples = buffer.getNumSamples();

  // Process each sample
  for (int sample = 0; sample < numSamples; ++sample)
  {
    // Read from delay buffer (stereo interleaved)
    int readPos = writePosition - delaySamples * 2;
    if (readPos < 0) readPos += delayBufferSize;

    float leftDelay = delayBuffer.getReadPointer(0)[readPos];
    float rightDelay = delayBuffer.getReadPointer(1)[readPos];

    // Input samples
    float leftIn = leftInput[sample];
    float rightIn = rightInput[sample];

    // Feedback: mix delay signal back into input
    float leftOut = leftIn + leftDelay * currentFeedback;
    float rightOut = rightIn + rightDelay * currentFeedback;

    // Write to delay buffer
    delayBuffer.setSample(0, writePosition, leftOut);
    delayBuffer.setSample(1, writePosition, rightOut);

    // Move write position (stereo interleaved)
    writePosition += 1;
    if (writePosition >= delayBufferSize) writePosition = 0;

    // Dry / Wet mix
    float leftMixed = leftIn * (1.0f - currentDryWet) + leftDelay * currentDryWet;
    float rightMixed = rightIn * (1.0f - currentDryWet) + rightDelay * currentDryWet;

    // Write output
    leftOutput[sample] = leftMixed;
    rightOutput[sample] = rightMixed;
  }

}

//===============================================================================
juce::AudioProcessorEditor* DelayAudioProcessor::createEditor()
{
    return new DelayAudioProcessorEditor (*this);
}

bool DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

//==============================================================================

//==============================================================================
void DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = juce::ValueTree("state");
    state.setProperty("delayTime", delayTimeParam->get(), nullptr);
    state.setProperty("feedback", feedbackParam->get(), nullptr);
    state.setProperty("dryWet", dryWetParam->get(), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto xml = std::unique_ptr<juce::XmlElement>(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
    {
      auto state = juce::ValueTree::fromXml(*xml);
      if (state.isValid())
      {
        *delayTimeParam = state.getProperty("delayTime", 0.5f);
        *feedbackParam = state.getProperty("feedback", 0.3f);
        *dryWetParam = state.getProperty("dryWet", 0.5f);
        lastDelayTime = *delayTimeParam;
      }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}
