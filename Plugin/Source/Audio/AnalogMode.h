#pragma once
#include "AutoGain.h"
#include "JuceHeader.h"
#include "Structs.h"

#include <vector>

namespace VSTZ {

struct AnalogMode {
  Channel Apply(double inLeft, double inRight, double drive);
  void CalculateWarmEffect(float value);
  void SetVoltage(float value);

  void PreProcess(const std::vector<Channel> &buffer, size_t size);
  void PostProcess(std::vector<Channel> &buffer, size_t size);
  double Tube(double in, size_t index, double drive);

  void SetSampleRate(double sampleRate);
  void SetBufferSize(size_t size);

private:
  double m_DistortionAmount{0.0f};
  double m_Voltage{160.0f};
  size_t m_CurrentProvidedSamples{0};

  DCFilter m_TubeFilter[2];
  DCFilter m_DryFilter[2];
  DCFilter m_DryFilterPre[2];
  AutoGainer m_AutoGain;
  std::vector<Channel> m_PreBuffer;
};
} // namespace VSTZ