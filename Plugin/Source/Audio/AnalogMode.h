#pragma once
#include "../Filter.h"
#include "JuceHeader.h"
#include "Structs.h"
#include "TargetDrive.h"

#include <vector>

namespace VSTZ {
struct AnalogChannel {
  double Left, Right;
};
struct AnalogMode {
  AnalogChannel ApplyPreDistortion(double inLeft, double inRight, size_t index);
  AnalogChannel ApplyPost(double inL, double inR);

  void Resize(size_t maxSize);
  void SetupFilter(double sR);
  void CalculateWarmEffect(float value);

  void PreProcess(const std::vector<Channel>&buffer, size_t size);

  TargetDrive DriveTarget;

  Filter m_AnalogFilter{};
  Filter m_SmoothFilter{};
  double m_DistortionAmount{0.0f};
  double m_Gain{-1.0f};

  std::vector<double> m_BufferLeft;
  std::vector<double> m_BufferRight;
  size_t m_CurrentProvidedSamples{0};
};
} // namespace VSTZ