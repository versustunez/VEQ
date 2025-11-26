#include "AnalogMode.h"

#include "Utils/Audio.h"

#include <JuceHeader.h>

namespace VSTZ {

static double lerp(const double a, const double b, const double alpha) {
  return a + alpha * (b - a);
}

Channel AnalogMode::Apply(const double inLeft, const double inRight, double drive) {
  return {
      Tube(inLeft, 0, drive),
      Tube(inRight, 1, drive),
  };
}

void AnalogMode::CalculateWarmEffect(const float value) {
  m_DistortionAmount = value;
}
void AnalogMode::SetVoltage(float value) { m_Voltage = lerp(100, 300, value); }
void AnalogMode::PreProcess(const std::vector<Channel> &buffer, size_t size) {
  m_CurrentProvidedSamples = size;
  for (size_t i = 0; i < m_CurrentProvidedSamples; i++) {
    m_PreBuffer[i] = {
        m_DryFilterPre[0].Process(buffer[i].Left),
        m_DryFilterPre[1].Process(buffer[i].Right),
    };
  }
  m_AutoGain.providePreSamples(m_PreBuffer, size);
}

void AnalogMode::SetSampleRate(double sampleRate) {
  m_AutoGain.setSampleRate(sampleRate);
  constexpr size_t numChannels = 2;
  for (size_t i = 0; i < numChannels; i++) {
    m_DryFilterPre[i].SetSampleRate(sampleRate);
    m_DryFilter[i].SetSampleRate(sampleRate);
    m_TubeFilter[i].SetSampleRate(sampleRate);
  }
}
void AnalogMode::SetBufferSize(size_t size) { m_PreBuffer.resize(size); }
void AnalogMode::PostProcess(std::vector<Channel> &buffer, size_t size) {
  m_AutoGain.providePostSamples(buffer, size);
  for (size_t i = 0; i < size; i++) {
    Channel red = m_AutoGain.gainReduction();
    buffer[i] *= red;
  }
}

double AnalogMode::Tube(double in, size_t index, double drive) {
  double x = in * drive;
  double bias = m_Voltage / 500.0;
  double shifted = x - bias;
  double term = shifted;
  double y = term + 0.1 * term*term - 0.05 * term*term*term;
  y *= 0.8;
  y += 0.03 * term * fabs(term);
  y = m_TubeFilter[index].Process(y);
  double dry = m_DryFilter[index].Process(in);
  return dry + y * m_DistortionAmount;
}

} // namespace VSTZ
