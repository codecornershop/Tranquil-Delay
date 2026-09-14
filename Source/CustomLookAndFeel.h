#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
  CustomLookAndFeel();

  void drawRotarySlider (juce::Graphics& g,
      int x, int y, int width, int height,
      float sliderPos,
      float rotaryStartAngle,
      float rotaryEndAngle,
      juce::Slider& slider) override;

  void drawLabel(juce::Graphics& g,
      juce::Label& label) override;

  void drawButtonBackground(juce::Graphics& g,
      juce::Button& button,
      const juce::Colour& backgroundColour,
      bool isMouseOver,
      bool isButtonDown) override;

  void drawToggleButton (juce::Graphics& g,
      juce::ToggleButton& button,
      bool shouldDrawButtonAsHighlighted,
      bool shouldDrawButtonAsDown) override;
};
