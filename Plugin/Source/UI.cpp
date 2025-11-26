#include "UI.h"

#include "Core/Config.h"
#include "Editor/EQUI.h"
#include "GUI/Theme/Theme.h"

namespace VSTZ {

constexpr double tabHeight = 40.0;
constexpr double tabHeightHalf = tabHeight * 0.5;
constexpr static int EQHeight = 80;

void UI::Init() {
  m_Instance = Core::Instance::get(m_ID);
  m_Bypass.Create("bypass", "Bypass", m_ID);
  m_Bypass->setButtonText("Bypass");

  m_MidSide.Create("mid_side", "Mid/Side", m_ID);
  m_MidSide->setButtonText("Mid/Side");

  m_WarmthStrength.Create("analog_strength", "Strength", m_ID);
  m_Voltage.Create("analog_voltage", "Voltage", m_ID);

  m_Drive.Create("drive", "Drive", m_ID);

  m_Logo.Create(m_ID);
  m_EQUI.Create(m_ID, -1);
  m_SpectrumBefore.Create(m_ID, &m_Instance->InputFFT);
  m_SpectrumAfter.Create(m_ID, &m_Instance->OutputFFT);
  // get Colors from config
  m_SpectrumBefore->SetColor(
      Core::Config::get().theme()->getColor(Theme::Colors::accent).darker(0.7));
  m_SpectrumAfter->SetColor(
      Core::Config::get().theme()->getColor(Theme::Colors::accent));

  m_FrequencyResponse.Create(m_ID);

  m_FrequencyPad.Create(m_ID);
  m_FrequencyPad->SetEQUI(m_EQUI.Get());

  m_DecibelMeter.Create(m_ID);

  addAndMakeVisible(*m_Logo);
  addAndMakeVisible(*m_Bypass);
  addAndMakeVisible(*m_MidSide);
  addAndMakeVisible(*m_WarmthStrength);
  addAndMakeVisible(*m_Voltage);
  addAndMakeVisible(*m_Drive);
  addAndMakeVisible(*m_SpectrumBefore);
  addAndMakeVisible(*m_SpectrumAfter);
  addAndMakeVisible(*m_DecibelMeter);
  addAndMakeVisible(*m_FrequencyResponse);
  addAndMakeVisible(*m_FrequencyPad);
  addChildComponent(*m_EQUI);
}

#define SetBoundAndIncrement(component, y, width, height)                      \
  component->setBounds(x, y, width, height);                                   \
  x += width

void UI::resized() {

  m_Logo->setBounds(5, 5, 100, tabHeight-10);
  float x = getWidth() - 260.0f;
  SetBoundAndIncrement(m_Drive, 0, 60, tabHeight);
  m_WarmthStrength->setBounds(x, 0, 60, tabHeightHalf);
  SetBoundAndIncrement(m_Voltage, tabHeightHalf, 60, tabHeightHalf);
  SetBoundAndIncrement(m_MidSide, 0, 70, tabHeight);
  SetBoundAndIncrement(m_Bypass, 0, 70, tabHeight);

  int specHeight = getHeight() - tabHeight;
  int gap = 20;
  juce::Rectangle<int> newBounds{gap, (int)tabHeight, getWidth() - gap,
                                 specHeight};
  m_SpectrumBefore->setBounds(newBounds);
  m_SpectrumAfter->setBounds(newBounds);
  m_FrequencyResponse->setBounds(newBounds);
  m_FrequencyPad->setBounds(newBounds);
  m_DecibelMeter->setBounds(0, 40, getWidth(), specHeight);

  {
    float specWidth = 360;
    float specX = (getWidth() - specWidth) * 0.5;
    float specH = specHeight + 20;
    m_EQUI->setBounds(specX, specH - EQHeight, specWidth, EQHeight);
  }
}

#undef SetBoundAndIncrement;

void UI::paint(juce::Graphics &g) {
  g.setColour(juce::Colour(0.0f, 0.0f, 0.0f, .2f));
  g.fillRect(0, 0, getWidth(), tabHeight);

  if (m_Instance->state.TrackColor) {
    auto color = *m_Instance->state.TrackColor;
    m_SpectrumBefore->SetColor(color.darker(0.7));
    m_SpectrumAfter->SetColor(color);
  }
}

void UI::handleAsyncUpdate() {
  setBounds(0, 0, getParentWidth(), getParentHeight());
}
} // namespace VSTZ
