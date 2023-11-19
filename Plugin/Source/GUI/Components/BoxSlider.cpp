#include <Core/Config.h>
#include <Core/Instance.h>
#include <GUI/Components/BoxSlider.h>
#include <VUtils/StringUtils.h>
#include <utility>

namespace VSTZ::GUI {
BoxSlider::BoxSlider(std::string name, std::string showName, InstanceID id)
    : BaseComponent(std::move(name), std::move(showName), id) {
  m_labelPosition = NO_LABEL;
  m_slider.Create();
  m_slider->setSliderStyle(juce::Slider::LinearBar);
  m_slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  m_slider->setComponentID(std::to_string(id));
  m_slider->setScrollWheelEnabled(true);

  // get instance and try to add Slider Attachments
  auto *instance = Core::Instance::get(m_id);
  if (instance && instance->treeState) {
    auto state = instance->treeState;
    m_Parameter = state->getParameter(m_name);
    if (m_Parameter != nullptr) {
      m_attachment =
          std::make_unique<SliderAttachment>(*state, m_name, *m_slider);
    }
  }
  addAndMakeVisible(*m_slider);
}
void BoxSlider::paint(juce::Graphics &) {}
void BoxSlider::resized() {
  m_slider->setBounds(0, 0, getWidth(), getHeight());
}
BoxSlider::~BoxSlider() = default;
juce::Slider *BoxSlider::slider() { return m_slider.Get(); }
void BoxSlider::setTooltip(const bool tooltip) const {
  m_slider->setPopupDisplayEnabled(false, tooltip, getParentComponent(), 500);
}
} // namespace VSTZ::GUI
