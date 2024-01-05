#pragma once
#include "../Filter.h"

namespace VSTZ {
struct AnalogChannel {
  double Left, Right;
};
struct AnalogMode {
  AnalogChannel ApplyPreDistortion(double inLeft, double inRight);
  AnalogChannel ApplyPostDistortion(double inL, double inR);

  void SetupFilter(double sR);
  void CalculateWarmEffect(float value);
  void ResetSlew(float left, float right);

  Filter m_AnalogFilter{};
  double m_DistortionAmount{0.0f};
  double m_AnalogSlew{0.0f};

  double m_LastValueLeft{0}, m_LastValueRight{0};
};
} // namespace VSTZ