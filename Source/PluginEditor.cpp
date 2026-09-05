#include "PluginProcessor.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    delayTimeSlider.textFromValueFunction = [this] (double value) {
      if (*audioProcessor.tempoSyncParam) {
        int index = (int)*audioProcessor.syncDivisionParam;
        const char* divisionNames[] = {
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
        };
        return juce::String(divisionNames[index]);
      } else {
        return juce::String(value, 2) + "s";
      }
    };
    delayTimeSlider.setRange(0.01f, 2.0f, 0.001f);
    delayTimeSlider.setValue(*audioProcessor.delayTimeParam);
    addAndMakeVisible(delayTimeSlider);

    feedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,  80, 20);
    feedbackSlider.setRange(0.0f, 0.95f, 0.001f);
    feedbackSlider.setValue(*audioProcessor.feedbackParam);
    addAndMakeVisible(feedbackSlider);

    dryWetSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,  80, 20);
    dryWetSlider.setRange(0.0f, 1.0f, 0.001f);
    dryWetSlider.setValue(*audioProcessor.dryWetParam);
    addAndMakeVisible(dryWetSlider);

    tempoSyncButton.setButtonText("Sync");
    tempoSyncButton.setToggleState(false, juce::dontSendNotification);
    tempoSyncButton.onClick = [this] {
      // Update the parameter when clicked
      *audioProcessor.tempoSyncParam = tempoSyncButton.getToggleState();
      delayTimeSlider.updateText();
    };
    addAndMakeVisible(tempoSyncButton);

    delayTimeLabel.setText("Delay", juce::dontSendNotification);
    delayTimeLabel.attachToComponent(&delayTimeSlider, false);
    addAndMakeVisible(delayTimeLabel);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider,  false);
    addAndMakeVisible(feedbackLabel);

    dryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent(&dryWetSlider, false);
    addAndMakeVisible(dryWetLabel);
    
    tempoSyncButton.setToggleState (*audioProcessor.tempoSyncParam, juce::dontSendNotification);
    tempoSyncButton.onClick = [this] {
      bool isSyncOn = tempoSyncButton.getToggleState();
      *audioProcessor.tempoSyncParam = isSyncOn;
    };

    clearBufferButton.setButtonText("Clear");
    clearBufferButton.setToggleState(false, juce::dontSendNotification);
    clearBufferButton.onClick = [this] {
      *audioProcessor.getClearBufferParam() = clearBufferButton.getToggleState();
    };
    addAndMakeVisible(clearBufferButton);

    clearBufferButton.setToggleState(*audioProcessor.getClearBufferParam(), juce::dontSendNotification);

    delayTimeSlider.onValueChange = [this] {
      float value = (float)delayTimeSlider.getValue();
      float knobPosition = delayTimeSlider.getNormalisableRange().convertTo0to1((float)delayTimeSlider.getValue());
      
      if (*audioProcessor.tempoSyncParam) {
        int numDivisions = 12;
        int index = (int)(knobPosition * (numDivisions - 1) + 0.5f);
        index = juce::jlimit(0, numDivisions - 1, index);
        float snappedValue = ((float)index / (numDivisions - 1) * 2.0f);
        delayTimeSlider.setValue(snappedValue, juce::dontSendNotification);

        *audioProcessor.syncDivisionParam = index;
      } else {
        *audioProcessor.delayTimeParam = value;
      }
    };
    feedbackSlider.onValueChange = [this] {
      *audioProcessor.feedbackParam = feedbackSlider.getValue();
    };
    dryWetSlider.onValueChange = [this] {
      *audioProcessor.dryWetParam = dryWetSlider.getValue();
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 300);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
}

//==============================================================================
void DelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour (0xff323232));
}

void DelayAudioProcessorEditor::resized()
{
  // Position sliders in a row
  auto area = getLocalBounds().reduced (10);

  auto knobArea = area.removeFromTop (100);
  int knobWidth = knobArea.getWidth() / 3;

  delayTimeSlider.setBounds (knobArea.removeFromLeft (knobWidth).reduced (10));
  feedbackSlider.setBounds (knobArea.removeFromLeft (knobWidth).reduced (10));
  dryWetSlider.setBounds (knobArea.reduced (10));

  auto controlArea = area.removeFromTop (40);
  auto syncButtonArea = controlArea.removeFromLeft (80);
  auto clearButtonArea = controlArea.removeFromLeft(160);

  clearBufferButton.setBounds(clearButtonArea);
  tempoSyncButton.setBounds (syncButtonArea);
}
