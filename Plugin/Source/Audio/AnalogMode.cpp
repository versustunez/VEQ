#include "AnalogMode.h"

#include <JuceHeader.h>
#include <cmath>

namespace VSTZ {

static double lerp(const double a, const double b, const double alpha) {
  return a + alpha * (b - a);
}

AnalogChannel AnalogMode::Apply(const double inLeft,
                                             const double inRight,
                                             size_t index) {
  DriveTarget.Update();
  const double drive = DriveTarget.GetDrive();
  const double reduction = DriveTarget.GetDriveReduction();
  return {lerp(inLeft,
               Tube(m_BufferLeft[index], drive, reduction),
               m_DistortionAmount),
          lerp(inRight,
               Tube(m_BufferRight[index], drive, reduction),
               m_DistortionAmount)};
}

void AnalogMode::Resize(const size_t maxSize) {
  m_BufferLeft.resize(maxSize);
  m_BufferRight.resize(maxSize);
}

void AnalogMode::SetupFilter(const double sR) {
  m_AnalogFilter.SetSampleRate(sR);
  m_AnalogFilter.SetFilterType(Filter::Type::LowPass);
  // the AnalogFilter needs to cut to avoid Aliasing ;)
  m_AnalogFilter.CalculateCoefficients(48.0, 500, 0.707);
}

void AnalogMode::CalculateWarmEffect(const float value) {
  m_DistortionAmount = lerp(0.01, 0.3, value);
}
void AnalogMode::SetVoltage(float value) {
  m_Voltage = lerp(100, 300, value);
}
void AnalogMode::PreProcess(const std::vector<Channel> &buffer, size_t size) {
  m_CurrentProvidedSamples = size;
  for (int i = 0; i < m_CurrentProvidedSamples; ++i) {
    auto &buf = buffer[i];
    m_BufferLeft[i] = m_AnalogFilter.ApplyLeft(buf.Left);
    m_BufferRight[i] = m_AnalogFilter.ApplyRight(buf.Right);
  }
  DriveTarget.CalculateDrive(m_BufferLeft.data(), m_BufferRight.data(),
                             m_CurrentProvidedSamples);
  for (int i = 0; i < m_CurrentProvidedSamples; ++i) {
    auto &buf = buffer[i];
    auto [Left, Right] = Apply(buf.Left, buf.Right, i);
    m_BufferLeft[i] = Left;
    m_BufferRight[i] = Right;
  }
}

double AnalogMode::Tube(double in, double drive, double reduction) const {
  const double mu = 70.0;
  const double kp = 600.0;
  const double kg1 = 1060.0;
  const double ex = 1.4;
  constexpr double inVolt = 5.0;
  constexpr double outVolt = (1.0 / inVolt) / 1000;


  const double vgk = in * inVolt * drive;

  double term = abs(vgk + m_Voltage / mu);
  double res = kp * pow(term / (1.0 + (term / kg1)), ex);

  return res * outVolt  * (vgk < 0 ? -1 : 1) * reduction;
}

} // namespace VSTZ
