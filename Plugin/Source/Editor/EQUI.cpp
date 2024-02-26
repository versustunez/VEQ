#include "EQUI.h"

#include "Core/Instance.h"

#include <FMT.h>

namespace VSTZ::Editor {
EQUI::~EQUI() { Uninit(); }

void EQUI::Init() {
  {
    auto name = fmt::format("Band{}_type", m_Index + 1);
    m_Type.Create(name, "Type", m_ID);
  }
  {
    auto name = fmt::format("Band{}_freq", m_Index + 1);
    m_Frequency.Create(name, "Frequency", m_ID);
  }
  {
    auto name = fmt::format("Band{}_gain", m_Index + 1);
    m_Gain.Create(name, "Gain", m_ID);
  }
  {
    auto name = fmt::format("Band{}_q", m_Index + 1);
    m_Q.Create(name, "Q", m_ID);
  }
  auto buttons = {m_Type.Get(), m_Frequency.Get(), m_Gain.Get(), m_Q.Get()};
  for (auto b : buttons) {
    b->enableLiveLabel(true);
    addAndMakeVisible(b);
  }
  resized();
}

void EQUI::resized() {
  if (!m_Type)
    return;
  juce::FlexBox fb;
  fb.flexWrap = juce::FlexBox::Wrap::noWrap;
  fb.justifyContent = juce::FlexBox::JustifyContent::center;
  fb.alignItems = juce::FlexBox::AlignItems::center;
  auto buttons = {m_Type.Get(), m_Frequency.Get(), m_Gain.Get(), m_Q.Get()};
  for (auto *b : buttons) {
    if (!b->isVisible())
      continue;
    fb.items.add(juce::FlexItem(*b)
                     .withMinWidth(80.0f)
                     .withHeight(TabHeight)
                     .withMargin(juce::FlexItem::Margin(5.0f)));
  }

  fb.performLayout(getLocalBounds());
}

void EQUI::paint(juce::Graphics &graphics) {
  graphics.setColour(juce::Colour::fromRGBA(21, 21, 26, 240));
  graphics.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 20.0f);
}

void EQUI::Hide() { setVisible(false); }

void EQUI::SwitchTo(int index) {
  if (index == -1) {
    m_Index = index;
    Hide();
    return;
  }
  setVisible(true);
  if (m_Index == index)
    return;
  m_Index = index;
  Uninit();
  Init();
}

void EQUI::Uninit() {
  // We are already Not Init...
  if (m_Type == nullptr)
    return;

  removeChildComponent(m_Type.Get());
  removeChildComponent(m_Frequency.Get());
  removeChildComponent(m_Gain.Get());
  removeChildComponent(m_Q.Get());

  // Reset ;)
  m_Type = nullptr;
  m_Frequency = nullptr;
  m_Gain = nullptr;
  m_Q = nullptr;
}
} // namespace VSTZ::Editor
