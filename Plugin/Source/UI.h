#pragma once

#include "Core/Instance.h"
#include "Editor/FrequencyResponse.h"
#include "Editor/Spectrum.h"
#include "Editor/XYPad.h"

#include <GUI/Components/Components.h>
#include <TypeDefs.h>

namespace VSTZ {

class Logo : public GUI::VComponent {
public:
  explicit Logo(InstanceID id);
  void paint(juce::Graphics &g) override;

protected:
  InstanceID m_ID;
};

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
  Scope<Logo> m_Logo;
  Scope<Editor::Spectrum> m_SpectrumLeft;
  Scope<Editor::Spectrum> m_SpectrumRight;
  Scope<Editor::FrequencyResponse> m_FrequencyResponse;
  Scope<Editor::XYPad> m_FrequencyPad;
  Scope<juce::TabbedComponent> m_Tab;

  Core::Instance *m_Instance;
};
} // namespace VSTZ
