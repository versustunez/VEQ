#include "UI.h"

#include "Core/Config.h"
#include "Editor/EQUI.h"
#include "GUI/Theme/Theme.h"

namespace VSTZ {

constexpr static int TabHeight = 80;

void UI::Init() {
  m_Instance = Core::Instance::get(m_ID);
  m_Bypass.Create("bypass", "Bypass", m_ID);
  m_Bypass->setButtonText("Bypass");

  m_AutoGain.Create("auto_gain", "Auto Gain", m_ID);
  m_AutoGain->setButtonText("Auto Gain");

  m_Warmth.Create("analog", "Analog", m_ID);
  m_Warmth->setButtonText("Analog");

  m_WarmthStrength.Create("analog_strength", "Strength", m_ID);

  m_DriveMeter.Create(&m_Instance->Processor->m_AnalogMode.DriveTarget);

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
  addAndMakeVisible(*m_AutoGain);
  addAndMakeVisible(*m_Warmth);
  addAndMakeVisible(*m_WarmthStrength);
  addAndMakeVisible(*m_DriveMeter);
  addAndMakeVisible(*m_SpectrumBefore);
  addAndMakeVisible(*m_SpectrumAfter);
  addAndMakeVisible(*m_DecibelMeter);
  addAndMakeVisible(*m_FrequencyResponse);
  addAndMakeVisible(*m_FrequencyPad);
  addChildComponent(*m_EQUI);
}

void UI::resized() {
  m_Bypass->setBounds(getWidth() - 70, 0, 70, 40);
  m_AutoGain->setBounds(getWidth() - 140, 0, 70, 40);
  m_Warmth->setBounds(getWidth() - 210, 0, 70, 20);
  m_WarmthStrength->setBounds(getWidth() - 210, 20, 70, 20);
  m_DriveMeter->setBounds(getWidth() - 220, 0, 10, 40);
  m_Logo->setBounds(5, 5, 100, 30);
  int specHeight = getHeight() - 40;
  juce::Rectangle<int> newBounds{20, 40, getWidth() - 20, specHeight};
  m_SpectrumBefore->setBounds(newBounds);
  m_SpectrumAfter->setBounds(newBounds);
  m_FrequencyResponse->setBounds(newBounds);
  m_FrequencyPad->setBounds(newBounds);
  m_DecibelMeter->setBounds(0, 40, getWidth(), specHeight);

  {
    float specWidth = 360;
    float specX = (getWidth() - specWidth) * 0.5;
    float specH = specHeight + 20;
    m_EQUI->setBounds(specX, specH - TabHeight, specWidth, TabHeight);
  }
}

void UI::paint(juce::Graphics &g) {
  g.setColour(juce::Colour(0.0f, 0.0f, 0.0f, .2f));
  g.fillRect(0, 0, getWidth(), 40);
}

void UI::handleAsyncUpdate() {
  setBounds(0,0,getParentWidth(), getParentHeight());
}
} // namespace VSTZ
