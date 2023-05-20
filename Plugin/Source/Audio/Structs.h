#pragma once

#include <algorithm>

namespace VSTZ {
struct Channel {
  double Left{0.0};
  double Right{0.0};
  double GetMax() { return std::max(Left, Right); }
  double GetMin() { return std::min(Left, Right); }
  double GetMaxAbs() { return std::max(std::abs(Left), std::abs(Right)); }
  Channel operator*=(const Channel &other) {
    Left *= other.Left;
    Right *= other.Right;
    return *this;
  }
  Channel operator+=(const Channel &other) {
    Left += other.Left;
    Right += other.Right;
    return *this;
  }
  Channel operator/=(const float other) {
    Left /= other;
    Right /= other;
    return *this;
  }
  Channel operator*=(double other) {
    Left *= other;
    Right *= other;
    return *this;
  }
  Channel operator+=(double other) {
    Left += other;
    Right += other;
    return *this;
  }
  Channel operator-(const Channel &other) const {
    return {Left - other.Left, Right - other.Right};
  }
  Channel operator*(const Channel &other) const {
    return {Left * other.Left, Right * other.Right};
  }
};
}; // namespace VSTZ