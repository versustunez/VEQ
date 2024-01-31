#pragma once

namespace VUtils {
template <typename T> struct MathImpl {
  static T Map(T t, T inMin, T inMax, T outMin, T outMax) {
    return (t - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  }
};

using Math = MathImpl<double>;
using MathF = MathImpl<float>;
} // namespace VUtils
