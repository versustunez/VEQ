#include <Core/Config.h>
#include <GUI/Theme/LookAndFeel/FlatLook.h>
#include <GUI/Theme/Theme.h>

namespace VSTZ::GUI {
typedef VSTZ::Theme::Colors Colors;

static constexpr float cornerRadius = 2.5f;
FlatLook::FlatLook() { theme = Core::Config::get().theme().Get(); }

FlatLook::~FlatLook() { theme = nullptr; }

void FlatLook::drawButtonBackground(juce::Graphics &graphics,
                                    juce::Button &button, const juce::Colour &,
                                    bool shouldDrawButtonAsHighlighted, bool) {
  const auto buttonArea = button.getLocalBounds();
  if (shouldDrawButtonAsHighlighted)
    graphics.setColour(theme->getColor(Theme::Colors::accent).withAlpha(0.3f));
  else
    graphics.setColour(juce::Colours::black.withAlpha(0.2f));
  graphics.fillRect(buttonArea.getX(), buttonArea.getY(), buttonArea.getWidth(),
                    buttonArea.getHeight());
}

void FlatLook::drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                                int height, float sliderPosProportional,
                                float rotaryStartAngle, float rotaryEndAngle,
                                juce::Slider &) {
  constexpr float size = 3.5f;
  constexpr float MAX_RADIAN = 2.53073;

  const auto widthF = static_cast<float>(width);
  const auto heightF = static_cast<float>(height);
  const auto xF = static_cast<float>(x);
  const auto yF = static_cast<float>(y);

  const float radius = juce::jmin(widthF / 2.0f, heightF / 2.0f) - 3.5f;
  const auto centreX = xF + widthF * 0.5f;
  const auto centreY = yF + heightF * 0.5f;
  const float arcPos = sliderPosProportional;
  const auto angle = rotaryStartAngle + sliderPosProportional *
                                            (rotaryEndAngle - rotaryStartAngle);

  //---[the real draw]---//
  juce::Path outerArc;
  outerArc.addCentredArc(0, 0, radius, radius, 0, -MAX_RADIAN, MAX_RADIAN,
                         true);
  outerArc.applyTransform(juce::AffineTransform().translated(centreX, centreY));
  g.setColour(theme->getColor(Colors::accent).withAlpha(0.2f));
  g.strokePath(outerArc, juce::PathStrokeType(size));

  // prepare pointer for drawing
  juce::Path arc;
  const float radians =
      std::clamp(arcPos * 2.0f - 1.0f, -1.0f, 1.0f) * MAX_RADIAN;
  arc.addCentredArc(0, 0, radius, radius, 0, -MAX_RADIAN, radians, true);
  arc.applyTransform(juce::AffineTransform().translated(centreX, centreY));
  const auto colourTwo = theme->getColor(Colors::accentTwo);
  const auto colourMain = theme->getColor(Colors::accent);
  g.setGradientFill(juce::ColourGradient::horizontal(
      colourTwo, centreX - radius, colourMain, centreX + radius));
  g.strokePath(arc, juce::PathStrokeType(size));

  juce::Path pointer;
  constexpr auto pointerThickness = 3;
  constexpr int radiusOffset = 5;
  constexpr float pointerPlacement_X = -pointerThickness * 0.25f;
  float const pointerPlacement_Y = -(radius - radiusOffset);
  pointer.addEllipse(pointerPlacement_X, pointerPlacement_Y, pointerThickness,
                     pointerThickness);
  pointer.applyTransform(
      juce::AffineTransform::rotation(angle).translated(centreX, centreY));
  g.setColour(theme->getColor(Colors::accent));
  g.fillPath(pointer);
}

void FlatLook::drawTextEditorOutline(juce::Graphics &graphics, int width,
                                     int height, juce::TextEditor &) {

  const auto widthF = static_cast<float>(width);
  const auto heightF = static_cast<float>(height);
  using juce::ColourGradient;
  const auto colorOne = theme->getColor(Colors::accentTwo);
  const auto colorTwo = theme->getColor(Colors::accent);

  graphics.setGradientFill(
      ColourGradient::horizontal(colorOne, 0, colorTwo, widthF));
  const float widthOffset = widthF / 100.0f * 2.5f;
  graphics.drawLine(widthOffset, heightF, widthF - widthOffset, heightF, 1.3f);
}

void FlatLook::drawToggleButton(juce::Graphics &graphics,
                                juce::ToggleButton &button, bool,
                                const bool shouldDrawButtonAsDown) {
  const int height = button.getHeight();
  const int width = button.getWidth();
  auto text = button.getButtonText();
  const bool isPressed =
      shouldDrawButtonAsDown ||
      button.getToggleStateValue() == juce::ToggleButton::buttonDown;

  if (text == "")
    text = isPressed ? "On" : "Off";

  const auto onColor = theme->getColor(Colors::accent).withAlpha(0.5f);
  const auto offColor = theme->getColor(Colors::accentTwo).withAlpha(0.2f);
  graphics.setColour(isPressed ? onColor : offColor);

  constexpr int marginX = 0;
  constexpr int marginY = 0;
  constexpr int marginW = marginX * 2;
  constexpr int marginH = marginY * 2;

  graphics.fillRect(marginX, 0, width - marginW, height);
  graphics.setColour(theme->getColor(Colors::font));
  graphics.setFont(11);
  graphics.drawText(text, marginX, marginY, width - marginW, height - marginH,
                    juce::Justification::centred, true);
}

void FlatLook::drawTabButton(juce::TabBarButton &button,
                             juce::Graphics &graphics, bool, bool) {
  const juce::Rectangle activeArea(button.getActiveArea());
  graphics.setColour(theme->getColor(Colors::bg));
  graphics.fillRect(activeArea);

  const int width = button.getWidth();
  const int height = button.getHeight() - 1;
  const auto color = theme->getColor(Colors::accent);
  graphics.setColour(button.getToggleState() ? color : color.withAlpha(0.2f));
  constexpr auto heightOffset = 2;
  graphics.fillRect(0, height - heightOffset, width, heightOffset);
  graphics.setColour(theme->getColor(Colors::font));
  graphics.drawText(button.getButtonText(), 0, 0, width, height,
                    juce::Justification::centred, true);
}

void FlatLook::drawComboBox(juce::Graphics &graphics, const int width,
                            const int height, bool, int, int, int, int,
                            juce::ComboBox &) {
  constexpr auto borderHeight = 2;
  graphics.setColour(theme->getColor(Colors::accent));
  graphics.fillRect(0, height - borderHeight, width, borderHeight);
  graphics.setColour(theme->getColor(Colors::bgTwo).withAlpha(0.2f));
  graphics.fillRect(0, 0, width, height - borderHeight);
}

void FlatLook::drawLabel(juce::Graphics &graphics, juce::Label &label) {
  if (!label.isBeingEdited()) {
    const auto alpha = label.isEnabled() ? 1.0f : 0.5f;
    const juce::Font font(getLabelFont(label));
    graphics.setColour(
        label.findColour(juce::Label::textColourId).withAlpha(alpha));
    const auto textArea =
        getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
    graphics.drawFittedText(
        label.getText(), textArea, label.getJustificationType(),
        juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
        label.getMinimumHorizontalScale());
  }
}
void FlatLook::drawBubble(juce::Graphics &graphics, juce::BubbleComponent &,
                          const juce::Point<float> &,
                          const juce::Rectangle<float> &body) {
  graphics.setColour(theme->getColor(Colors::bg));
  graphics.fillRoundedRectangle(body, cornerRadius);
  graphics.setColour(theme->getColor(Colors::accent));
  graphics.drawRoundedRectangle(body, cornerRadius, 1.0f);
}
} // namespace VSTZ::GUI
