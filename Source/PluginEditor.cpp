#include "CustomLookAndFeel.h"
#include "PluginProcessor.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "PluginEditor.h"
#include "BinaryData.h"

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

    customLookAndFeel = std::make_unique<CustomLookAndFeel>();

    delayTimeSlider.setLookAndFeel(customLookAndFeel.get());
    delayTimeSlider.setRange(0.01f, 2.0f, 0.001f);
    delayTimeSlider.setValue(*audioProcessor.delayTimeParam);
    addAndMakeVisible(delayTimeSlider);

    feedbackSlider.setLookAndFeel(customLookAndFeel.get());
    feedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,  80, 20);
    feedbackSlider.setRange(0.0f, 0.95f, 0.001f);
    feedbackSlider.setValue(*audioProcessor.feedbackParam);
    addAndMakeVisible(feedbackSlider);

    dryWetSlider.setLookAndFeel(customLookAndFeel.get());
    dryWetSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,  80, 20);
    dryWetSlider.setRange(0.0f, 1.0f, 0.001f);
    dryWetSlider.setValue(*audioProcessor.dryWetParam);
    addAndMakeVisible(dryWetSlider);

    delayTimeLabel.setText("Delay", juce::dontSendNotification);
    delayTimeLabel.attachToComponent(&delayTimeSlider, false);
    delayTimeLabel.setLookAndFeel(customLookAndFeel.get());
    addAndMakeVisible(delayTimeLabel);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider,  false);
    feedbackLabel.setLookAndFeel(customLookAndFeel.get());
    addAndMakeVisible(feedbackLabel);

    dryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent(&dryWetSlider, false);
    dryWetLabel.setLookAndFeel(customLookAndFeel.get());
    addAndMakeVisible(dryWetLabel);

    tempoSyncButton.setButtonText("Sync");
    tempoSyncButton.setLookAndFeel(customLookAndFeel.get());
    tempoSyncButton.setToggleState(false, juce::dontSendNotification);
    tempoSyncButton.onClick = [this] {
      // Update the parameter when clicked
      *audioProcessor.tempoSyncParam = tempoSyncButton.getToggleState();
      delayTimeSlider.updateText();
    };
    addAndMakeVisible(tempoSyncButton);

    tempoSyncButton.setToggleState (*audioProcessor.tempoSyncParam, juce::dontSendNotification);
    tempoSyncButton.onClick = [this] {
      bool isSyncOn = tempoSyncButton.getToggleState();
      *audioProcessor.tempoSyncParam = isSyncOn;
    };
    
    clearBufferButton.setButtonText("Clear");
    clearBufferButton.setLookAndFeel(customLookAndFeel.get());
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

    static std::unique_ptr<juce::Drawable> logo;
    if (!logo)
    {
      logo = juce::Drawable::createFromImageData(
          BinaryData::logo_svg,
          BinaryData::logo_svgSize
          );
    }

    if (logo)
    {
      auto logoArea = juce::Rectangle<int> (10, 10, 80, 40);
      logo->drawWithin(g,
          logoArea.toFloat(),
          juce::RectanglePlacement::centred,
          1.0f);
    }
}

void DelayAudioProcessorEditor::resized()
{
  // Position sliders in a row
  auto area = getLocalBounds().reduced (10);

  const int topHeight = 50;
  auto topZone = area.removeFromTop(topHeight);
  
  const int logoWidth = 80;
  topZone.removeFromLeft(logoWidth);

  auto centreZone = area;

  auto knobArea = centreZone.removeFromTop (100);
  int knobWidth = knobArea.getWidth() / 3;

  delayTimeSlider.setBounds (knobArea.removeFromLeft (knobWidth).reduced (10));
  feedbackSlider.setBounds (knobArea.removeFromLeft (knobWidth).reduced (10));
  dryWetSlider.setBounds (knobArea.reduced (10));

  auto controlArea = area.removeFromTop (210);

  const int buttonWidth = 70;
  const int buttonHeight = 26;
  const int gap = 12;

  auto placeButton = [&](juce::Component& button)
  {
    auto slot = controlArea.removeFromLeft(buttonWidth);
    button.setBounds(slot.withSizeKeepingCentre(buttonWidth, buttonHeight));
    controlArea.removeFromLeft(gap);
  };

  placeButton(tempoSyncButton);
  placeButton(clearBufferButton);
}
