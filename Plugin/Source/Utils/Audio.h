#pragma once

#include <algorithm>
#include <cmath>

namespace VSTZ {

struct AudioUtils {
  static double DecibelToGain(double dB) {
    return std::pow(10.0, std::max(dB, -80.0) / 20.0);
  }
  static constexpr double RmsFloor = 1e-4;
  static double GainToDecibels(double gain) {
    return 20.0 * std::log10(std::max(gain, RmsFloor));
  }
};
} // namespace VSTZ