#include "AutoGain.h"

namespace VSTZ {
AutoGain::AutoGain() {
  for (int i = 0; i < BufferSize; i++) {
    m_In.Left[i] = 0;
    m_In.Right[i] = 0;
    m_Out.Left[i] = 0;
    m_Out.Right[i] = 0;
  }
  CalculateSmoothingFactor();
}

void AutoGain::CalculateSmoothingFactor() {
  constexpr double desiredTimeConstant = 0.05 / 1000.0;
  double numSamples = desiredTimeConstant * m_SampleRate;
  double scalingFactor = -1.0 / (numSamples * BufferSize);
  m_SmoothnessFactor = std::exp(scalingFactor);
}

void AutoGain::CalculateMakeupGain() {
  double rms = CalculateRMS(m_Out);
  double makeup = (rms > 0.0 && m_InputGain > 0.0) ? m_InputGain / rms : 1.0;
  m_Gain = m_SmoothnessFactor * m_Gain + (1.0f - m_SmoothnessFactor) * makeup;
  if ((makeup < m_Gain && m_Gain < 1.0) || m_Gain > 1.0 && makeup > m_Gain)
    m_Gain = makeup;
}

double AutoGain::CalculateRMS(RingBuffer &buffer) {
  double rmsL = 0.0;
  double rmsR = 0.0;
  for (size_t i = 0; i < BufferSize; i++) {
    rmsL += buffer.Left[i] * buffer.Left[i];
    rmsR += buffer.Right[i] * buffer.Right[i];
  }
  return std::sqrt(rmsL / BufferSize) + std::sqrt(rmsR / BufferSize);
}
} // namespace VSTZ