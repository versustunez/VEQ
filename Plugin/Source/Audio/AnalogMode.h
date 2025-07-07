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
  AnalogChannel Apply(double inLeft, double inRight, size_t index);

  void Resize(size_t maxSize);
  void SetupFilter(double sR);
  void CalculateWarmEffect(float value);
  void SetVoltage(float value);

  void PreProcess(const std::vector<Channel>&buffer, size_t size);

  TargetDrive DriveTarget;

  Filter m_AnalogFilter{};
  double m_DistortionAmount{0.0f};
  double m_Gain{-1.0f};
  double m_Voltage{160.0f}; // Harsher

  std::vector<double> m_BufferLeft;
  std::vector<double> m_BufferRight;
  size_t m_CurrentProvidedSamples{0};

  double Tube(double in, double drive, double reduction) const;
};
} // namespace VSTZ