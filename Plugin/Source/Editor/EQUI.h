#pragma once

#include "Events/Event.h"
#include "GUI/Components/Knob.h"
#include "GUI/Components/VComponent.h"
#include "TypeDefs.h"

namespace VSTZ::Editor {

constexpr static int TabHeight = 60;

class EQUI : public GUI::VComponent {
public:
  explicit EQUI(InstanceID id, int index = 1) : m_ID(id), m_Index(index) {}
  ~EQUI() override;
  void Init();
  void SwitchTo(int index);
  void Hide();
  void resized() override;
  void paint(juce::Graphics &graphics) override;
  int CurrentIndex() const { return m_Index; }

private:
  void Uninit();

protected:
  InstanceID m_ID{};
  int m_Index;
  Scope<GUI::Knob> m_Type;
  Scope<GUI::Knob> m_Frequency;
  Scope<GUI::Knob> m_Gain;
  Scope<GUI::Knob> m_Q;
};

} // namespace VSTZ::Editor
