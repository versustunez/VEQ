#pragma once

#include "Core/Instance.h"
#include "Editor/FrequencyResponse.h"
#include "Editor/Logo.h"
#include "Editor/Spectrum.h"
#include "Editor/XYPad.h"
#include "Editor/dBMeter.h"

#include <GUI/Components/BoxSlider.h>
#include <GUI/Components/Components.h>
#include <TypeDefs.h>

namespace VSTZ {

class UI : public GUI::VComponent {
public:
  explicit UI(InstanceID id) : m_ID(id) { Init(); }
  void Init();
  void resized() override;
  void paint(juce::Graphics &g) override;

protected:
  InstanceID m_ID{};
  Scope<GUI::Switch> m_Bypass;
  Scope<GUI::Switch> m_AutoGain;
  Scope<GUI::Switch> m_Warmth;
  Scope<GUI::BoxSlider> m_WarmthStrength;
  Scope<Editor::Logo> m_Logo;
  Scope<Editor::Spectrum> m_SpectrumBefore;
  Scope<Editor::Spectrum> m_SpectrumAfter;
  Scope<Editor::FrequencyResponse> m_FrequencyResponse;
  Scope<Editor::XYPad> m_FrequencyPad;
  Scope<juce::TabbedComponent> m_Tab;
  Scope<Editor::DecibelMeter> m_DecibelMeter;

  Core::Instance *m_Instance;
};
} // namespace VSTZ
