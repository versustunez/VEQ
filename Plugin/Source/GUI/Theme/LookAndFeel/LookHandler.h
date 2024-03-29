#pragma once

#include "FlatLook.h"

#include <JuceHeader.h>
#include <memory>

namespace VSTZ::GUI {
class LookHandler : public juce::LookAndFeel_V4, juce::DeletedAtShutdown {
private:
  int m_currentLook = 0;

public:
  LookHandler();

  ~LookHandler() override;
  void selectLook(int index);

  VeNoLook *getLook();

  void drawButtonBackground(juce::Graphics &g, juce::Button &button,
                            const juce::Colour &backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;

  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPosProportional, float rotaryStartAngle,
                        float rotaryEndAngle, juce::Slider &slider) override;

  void drawLinearSlider(juce::Graphics &graphics, int x, int y, int width,
                        int height, float sliderPos, float minSliderPos,
                        float maxSliderPos,
                        const juce::Slider::SliderStyle style,
                        juce::Slider &slider) override;

  void drawTextEditorOutline(juce::Graphics &graphics, int width, int height,
                             juce::TextEditor &editor) override;

  void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

  void drawTabButton(juce::TabBarButton &btn, juce::Graphics &g,
                     bool isMouseOver, bool isMouseDown) override;

  void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                    int buttonX, int buttonY, int buttonW, int buttonH,
                    juce::ComboBox &box) override;

  void drawLabel(juce::Graphics &graphics, juce::Label &label) override;

  void drawTabAreaBehindFrontButton(juce::TabbedButtonBar &bar,
                                    juce::Graphics &g, int w, int h) override;

  void drawPopupMenuItem(juce::Graphics &graphics,
                         const juce::Rectangle<int> &area, bool isSeparator,
                         bool isActive, bool isHighlighted, bool isTicked,
                         bool hasSubMenu, const juce::String &text,
                         const juce::String &shortcutKeyText,
                         const juce::Drawable *icon,
                         const juce::Colour *textColour) override;

  juce::PopupMenu::Options
  getOptionsForComboBoxPopupMenu(juce::ComboBox &box,
                                 juce::Label &label) override;

  void drawPopupMenuBackground(juce::Graphics &graphics, int width,
                               int height) override;

  void drawTooltip(juce::Graphics &graphics, const juce::String &text,
                   int width, int height) override;

  void drawBubble(juce::Graphics &graphics, juce::BubbleComponent &component,
                  const juce::Point<float> &tip,
                  const juce::Rectangle<float> &body) override;

  void drawButtonText(juce::Graphics &graphics, juce::TextButton &button,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

  void drawTabbedButtonBarBackground(juce::TabbedButtonBar &bar,
                                     juce::Graphics &graphics) override;

  void drawCallOutBoxBackground(juce::CallOutBox &box, juce::Graphics &graphics,
                                const juce::Path &path,
                                juce::Image &image) override;
  int getCallOutBoxBorderSize(const juce::CallOutBox &box) override;
  float getCallOutBoxCornerSize(const juce::CallOutBox &box) override;

  int getTabButtonBestWidth(juce::TabBarButton &button, int) override {
    return button.getTabbedButtonBar().getWidth() /
           button.getTabbedButtonBar().getNumTabs();
  }

  void drawLinearSliderBackground(juce::Graphics &graphics, int x, int y,
                                int width, int height, float sliderPos,
                                float minSliderPos, float maxSliderPos,
                                const juce::Slider::SliderStyle style,
                                juce::Slider &slider) override;

protected:
  void drawBasedOnKnob(juce::Graphics &g, int x, int y, int width, int height,
                       float sliderPosProportional, float rotaryStartAngle,
                       float rotaryEndAngle, juce::Slider &slider);

protected:
  // currently both available themes are CrazyLook <-- (this is a fun one xD)
  // and FlatLook
  VeNoLook *m_feels[1] = {new FlatLook()};
  VSTZ::Theme::Theme *theme = nullptr;
};

} // namespace VSTZ::GUI
