#pragma once

#include "BaseComponent.h"

#include <JuceHeader.h>
#include <memory>

// Forwards
namespace VSTZ::GUI {
class BoxSlider : public BaseComponent {
public:
  BoxSlider(std::string name, std::string showName, InstanceID id);
  ~BoxSlider() override;
  // Component
  void paint(juce::Graphics &g) override;
  void resized() override;

  juce::Slider *slider();

  void setMinMax(const double min, const double max,
                 const double interval = 0) const {
    m_slider->setRange(min, max, interval);
  }
  void setValue(const double value) const { m_slider->setValue(value); }
  void setDefaultValue(const double defaultValue) const {
    m_slider->setDoubleClickReturnValue(true, defaultValue);
  }
  [[nodiscard]] double getValue() const { return m_slider->getValue(); }
  void setTooltip(bool tooltip) const;

  [[nodiscard]] juce::RangedAudioParameter* GetParameter() const { return m_Parameter; }

protected:
  Scope<juce::Slider> m_slider;
  std::unique_ptr<SliderAttachment> m_attachment;
  juce::RangedAudioParameter* m_Parameter{nullptr};
};
} // namespace VSTZ::GUI