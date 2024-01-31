#pragma once
#include "Events/Event.h"
#include "GUI/Components/VComponent.h"
#include "PluginProcessor.h"

namespace VSTZ::Editor {
class DecibelMeter : public GUI::VComponent, Events::Handler {
public:
  explicit DecibelMeter(InstanceID id);
  ~DecibelMeter() override;
  void paint(juce::Graphics &g) override;

private:
  void Handle(Events::Event *event) override;
protected:
  struct Params {
    Core::Parameter *Type{nullptr};
    Core::Parameter *Frequency{nullptr};
    Core::Parameter *Gain{nullptr};
    Core::Parameter *Q{nullptr};
  };
protected:
  InstanceID m_ID{};
  std::array<Params, VSTProcessor::Bands> m_Params{};
  float m_Scale{6.0f};
};
} // namespace VSTZ::Editor