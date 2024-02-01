#pragma once
#include "../Filter.h"
#include "TargetDrive.h"

namespace VSTZ {
struct AnalogChannel {
  double Left, Right;
};
struct AnalogMode {
  AnalogChannel ApplyPreDistortion(double inLeft, double inRight);
  AnalogChannel ApplyPost(double inL, double inR);

  void SetupFilter(double sR);
  void CalculateWarmEffect(float value);

  TargetDrive DriveTarget;

  Filter m_AnalogFilter{};
  Filter m_SmoothFilter{};
  double m_DistortionAmount{0.0f};
  double m_Gain{-1.0f};
};
} // namespace VSTZ