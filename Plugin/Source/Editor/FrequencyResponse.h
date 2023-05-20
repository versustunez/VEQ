#pragma once

#include "BandListener.h"
#include "GUI/Components/VComponent.h"
#include "ParameterStruct.h"
#include "PluginProcessor.h"

namespace VSTZ::Editor {
class FrequencyResponse : public GUI::VComponent, Events::Handler {
public:
  explicit FrequencyResponse(InstanceID id);
  ~FrequencyResponse() override;
  void paint(juce::Graphics &g) override;
  void resized() override;

private:
  void PrepareResponse();
  void Handle(Events::Event *event) override;

protected:
  InstanceID m_ID{};
  std::array<juce::Path, VSTProcessor::Bands> m_ResponsePaths{};
};
} // namespace VSTZ::Editor
