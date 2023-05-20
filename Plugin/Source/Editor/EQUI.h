#pragma once

#include "Events/Event.h"
#include "GUI/Components/Knob.h"
#include "GUI/Components/VComponent.h"
#include "TypeDefs.h"

namespace VSTZ::Editor {

constexpr static int TabHeight = 120;

class EQUI : public GUI::VComponent, Events::Handler {
public:
  explicit EQUI(InstanceID id, int index = 1) : m_ID(id), m_Index(index) {
    Init();
  };
  ~EQUI() override;
  void Init();
  void resized() override;

private:
  void Handle(Events::Event *event) override;

protected:
  InstanceID m_ID{};
  int m_Index;
  Scope<GUI::Knob> m_Type;
  Scope<GUI::Knob> m_Frequency;
  Scope<GUI::Knob> m_Gain;
  Scope<GUI::Knob> m_Q;
};

} // namespace VSTZ::Editor
