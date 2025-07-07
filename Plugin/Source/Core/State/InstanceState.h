#pragma once

#include <JuceHeader.h>
#include <TypeDefs.h>
#include <optional>

namespace VSTZ::State {
class InstanceState {
public:
  explicit InstanceState(InstanceID id);
  InstanceID m_ID;

  std::optional<juce::Colour> TrackColor{};
};
} // namespace VSTZ::State