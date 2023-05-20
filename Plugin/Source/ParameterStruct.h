#pragma once

#include "Core/Parameter/Parameter.h"
#include "Filter.h"

namespace VSTZ {
struct Parameters {
  Core::Parameter *Bypass{nullptr};
  Core::Parameter *AutoGain{nullptr};
};
struct Band {
  Core::Parameter *Frequency{};
  Core::Parameter *Q{};
  Core::Parameter *Gain{};
  Core::Parameter *Type{};
  Filter ApplyingFilter;
};
} // namespace VSTZ
