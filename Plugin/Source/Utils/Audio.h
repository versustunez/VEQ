#pragma once

#include <cmath>
#include <algorithm>

namespace VSTZ {

struct AudioUtils {
  static double DecibelToGain(double dB) {
    return std::pow(10.0, std::max(dB, -70.0) / 20.0);
  }

  static double GainToDecibels(double gain) { return 20.0 * std::log10(gain); }
};
} // namespace VSTZ