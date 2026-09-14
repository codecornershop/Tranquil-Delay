#include "CustomLookAndFeel.h"
#include "juce_graphics/juce_graphics.h"

CustomLookAndFeel::CustomLookAndFeel()
{
  setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
  setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentWhite);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
  auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
  auto centre = bounds.getCentre();
  auto radius = juce::jmin(bounds.getWidth() / 2.0f, bounds.getHeight() / 2.0f) * 0.8f;

  // Background
  g.setColour(juce::Colour (0xff2a2a2a));
  g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

  // Outline
  g.setColour(juce::Colour (0xff444444));
  g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);

  // Arc (value indicator)
  auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
  juce::Path arcPath;
  arcPath.addArc(centre.x - radius * 0.7f,
      centre.y - radius * 0.7f,
      radius * 1.4f,
      radius * 1.4f,
      rotaryStartAngle,
      angle,
      true);
  g.setColour(juce::Colour (0xff4a8cff));
  g.strokePath(arcPath, juce::PathStrokeType (4.0f));

  // Tick
//  juce::Line<float> tickLine (
//      centre.x + radius * 0.6f * std::cos(angle),
//      centre.y + radius * 0.6f * std::sin(angle),
//      centre.x + radius * 0.8f * std::cos(angle),
//      centre.y + radius * 0.8f * std::sin(angle)
//      );
//  g.setColour(juce::Colours::white);
//  g.drawLine(tickLine, 2.0f);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool isMouseOver,
    bool isButtonDown)
{
  auto bounds = button.getLocalBounds().toFloat();

  if (isButtonDown)
    g.setColour(juce::Colour(0xff4a8cff));
  else if (isMouseOver)
    g.setColour(juce::Colour(0xff3a3a3a));
  else
    g.setColour(juce::Colour(0xff2a2a2a));

  g.fillRoundedRectangle(bounds, 4.0f);
  g.setColour(juce::Colours::white);
  g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void CustomLookAndFeel::drawLabel(juce::Graphics& g,
    juce::Label& label)
{
  g.setColour(juce::Colours::white);
  g.setFont(juce::Font (juce::FontOptions (14.0f).withStyle ("Bold")));
  g.drawText(label.getText(), label.getLocalBounds(),
      juce::Justification::centred, true);
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
  auto bounds = button.getLocalBounds().toFloat();

  if (shouldDrawButtonAsDown)
    g.setColour(juce::Colour(0xff3a3a3a));
  else if (shouldDrawButtonAsHighlighted)
    g.setColour(juce::Colour(0xff3c3c3c));
  else
    g.setColour(juce::Colour(0xff2a2a2a));

  g.fillRoundedRectangle(bounds, 4.0f);

  g.setColour(juce::Colour(0xff555555));
  g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

  if (button.getToggleState())
    g.setColour(juce::Colour(0xff4a8cff));
  else
    g.setColour(juce::Colours::white);

  g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
  g.drawText(button.getButtonText(),
      bounds,
      juce::Justification::centred,
      true);
}

