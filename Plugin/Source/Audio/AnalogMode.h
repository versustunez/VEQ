#pragma once
#include "../Filter.h"
#include "TargetDrive.h"

namespace VSTZ {
struct AnalogChannel {
  double Left, Right;
};
struct AnalogMode {
  AnalogChannel ApplyPreDistortion(double inLeft, double inRight);
  AnalogChannel ApplyPostDistortion(double inL, double inR);

  void SetupFilter(double sR);
  void CalculateWarmEffect(float value);


  TargetDrive DriveTarget;

  Filter m_AnalogFilter{};
  double m_DistortionAmount{0.0f};
  double m_Alpha{0.0f};
  double m_AlphaMix{0.0f};

  double m_LastValueLeft{0}, m_LastValueRight{0};
};
} // namespace VSTZ