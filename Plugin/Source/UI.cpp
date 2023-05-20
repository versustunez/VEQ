#include "UI.h"

#include "Editor/EQUI.h"

#include <FMT.h>

namespace VSTZ {

constexpr static int TabHeight = 120;

void UI::Init() {
  m_Instance = Core::Instance::get(m_ID);
  m_Bypass.Create("bypass", "Bypass", m_ID);
  m_Bypass->setButtonText("Bypass");

  m_AutoGain.Create("auto_gain", "Auto Gain", m_ID);
  m_AutoGain->setButtonText("Auto Gain");
  m_Logo.Create(m_ID);
  m_Tab.Create(juce::TabbedButtonBar::Orientation::TabsAtBottom);
  m_Tab->setOutline(0);

  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    auto *eq = new Editor::EQUI(m_ID, i + 1);
    m_Tab->addTab(std::to_string(i + 1), juce::Colour(0.0f, 0.0f, 0.0f, .2f),
                  eq, true);
  }
  m_SpectrumLeft.Create(m_ID, &m_Instance->LeftFFT);
  m_SpectrumRight.Create(m_ID, &m_Instance->RightFFT);
  m_SpectrumLeft->SetColor(0.2f, 0.3f, 1.0f);
  m_SpectrumRight->SetColor(0.0f, 1.0f, 0.3f);

  m_FrequencyResponse.Create(m_ID);

  m_FrequencyPad.Create(m_ID);
  m_FrequencyPad->SetTabbedComponents(m_Tab.Get());

  addAndMakeVisible(*m_Logo);
  addAndMakeVisible(*m_Bypass);
  addAndMakeVisible(*m_AutoGain);
  addAndMakeVisible(*m_SpectrumLeft);
  addAndMakeVisible(*m_SpectrumRight);
  addAndMakeVisible(*m_FrequencyResponse);
  addAndMakeVisible(*m_FrequencyPad);
  addAndMakeVisible(*m_Tab);
}
void UI::resized() {
  m_Bypass->setBounds(getWidth() - 70, 10, 50, 20);
  m_AutoGain->setBounds(getWidth() - 130, 10, 50, 20);
  m_Logo->setBounds(10, 5, getWidth(), 30);
  int specHeight = getHeight() - (TabHeight + 40);
  m_SpectrumLeft->setBounds(0, 40, getWidth(), specHeight);
  m_SpectrumRight->setBounds(0, 40, getWidth(), specHeight);
  m_FrequencyResponse->setBounds(0, 40, getWidth(), specHeight);
  m_FrequencyPad->setBounds(0, 40, getWidth(), specHeight);
  m_Tab->setBounds(0, getHeight() - TabHeight, getWidth(), TabHeight);
}

void UI::paint(juce::Graphics &g) {
  g.setColour(juce::Colour(0.0f, 0.0f, 0.0f, .2f));
  g.fillRect(0, 0, getWidth(), 40);
}

Logo::Logo(InstanceID id) : m_ID(id) {}
void Logo::paint(juce::Graphics &g) {

  g.setColour(juce::Colours::white);
  g.setFont(24);
  g.drawText(JucePlugin_Name, 0, 0, getWidth(), getHeight(),
             juce::Justification::centredLeft, true);
}
} // namespace VSTZ
