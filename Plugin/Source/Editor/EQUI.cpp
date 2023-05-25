#include "EQUI.h"

#include "Core/Instance.h"

#include <FMT.h>

namespace VSTZ::Editor {
EQUI::~EQUI() {
  Core::Instance::get(m_ID)->EventHandler.RemoveHandler(
      fmt::format("Band{}_type", m_Index), this);
}

void EQUI::Init() {
  auto instance = Core::Instance::get(m_ID);
  {
    auto name = fmt::format("Band{}_type", m_Index);
    m_Type.Create(name, "Type", m_ID);
  }
  {
    auto name = fmt::format("Band{}_freq", m_Index);
    m_Frequency.Create(name, "Frequency", m_ID);
  }
  {
    auto name = fmt::format("Band{}_gain", m_Index);
    m_Gain.Create(name, "Gain", m_ID);
  }
  {
    auto name = fmt::format("Band{}_q", m_Index);
    m_Q.Create(name, "Q", m_ID);
  }
  auto buttons = {m_Type.Get(), m_Frequency.Get(), m_Gain.Get(), m_Q.Get()};
  for (auto b : buttons) {
    b->enableLiveLabel(true);
    addAndMakeVisible(b);
  }
  int val = (int)m_Type->getValue();

  m_Q->setVisible(val == 2);
  m_Frequency->setVisible(val != 0);
  m_Gain->setVisible(val != 0);

  instance->EventHandler.AddHandler(fmt::format("Band{}_type", m_Index), this);
}

void EQUI::resized() {
  juce::FlexBox fb;
  fb.flexWrap = juce::FlexBox::Wrap::noWrap;
  fb.justifyContent = juce::FlexBox::JustifyContent::center;
  fb.alignItems = juce::FlexBox::AlignItems::center;
  auto buttons = {m_Type.Get(), m_Frequency.Get(), m_Gain.Get(), m_Q.Get()};
  for (auto *b : buttons) {
    if (!b->isVisible())
      continue;
    fb.items.add(juce::FlexItem(*b)
                     .withMinWidth(100.0f)
                     .withHeight(TabHeight - 40)
                     .withMargin(juce::FlexItem::Margin(5.0f)));
  }

  fb.performLayout(getLocalBounds());
}

void EQUI::Handle(Events::Event *event) {
  auto *pc = event->As<Events::ParameterChange>();
  if (!pc)
    return;

  int val = (int)pc->Parameter->getValue();

  m_Q->setVisible(val != 0);
  m_Frequency->setVisible(val != 0);
  m_Gain->setVisible(val != 0);

  resized();
  repaint();
}
} // namespace VSTZ::Editor