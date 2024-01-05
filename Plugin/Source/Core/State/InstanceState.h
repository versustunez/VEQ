#pragma once

#include <JuceHeader.h>
#include <TypeDefs.h>

namespace VSTZ::State {
class InstanceState {
public:
  explicit InstanceState(InstanceID id);
  InstanceID m_ID;

  juce::Colour TrackColor{};
};
} // namespace VSTZ::State