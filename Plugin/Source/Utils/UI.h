#pragma once

#include "Core/Parameter/Parameter.h"
#include "PluginProcessor.h"

namespace VSTZ::Utils {

class UI {
public:
  struct ScaleData {
    Core::Parameter* Gain;
    Core::Parameter* Type;
  };

  static float GetDecibelScale(const ScaleData& data);
  static float GetDecibelScaleForArray(const ScaleData *data, int size);
};

} // namespace VSTZ::Utils
