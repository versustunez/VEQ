#include "AnalogMode.h"

#include <JuceHeader.h>
#include <cmath>

namespace VSTZ {

static double lerp(const double a, const double b, const double alpha) {
  return a + alpha * (b - a);
}

static double ApplyAnalogDistortion(const double in, const double driveAmount,
                                    const double driveAmountReduction) {
  constexpr double curvePct = 0;
  constexpr double curveA = 1.5 + curvePct;
  constexpr double curveB = -(curvePct + curvePct);
  constexpr double curveC = curvePct - 0.5;
  constexpr double curveD =
      0.0625 - curvePct * 0.25 + (curvePct * curvePct) * 0.25;

  double input = in * driveAmount;
  const double sign = input > 0 ? 1 : -1;

  const double s1 = std::abs(input);
  const double s2 = s1 * s1;
  const double s3 = s2 * s1;
  const double s4 = s2 * s2;

  if (s1 >= 2.0) {
    input = 0;
  } else if (s1 > 1.0) {
    input = (2 * s1) - s2;
  } else {
    input = (curveA * s1) + (curveB * s2) + (curveC * s3) -
            (curveD * (s2 - (2.0 * s3) + s4));
  }
  return input * sign * driveAmountReduction;
}

AnalogChannel AnalogMode::ApplyPreDistortion(const double inLeft,
                                             const double inRight,
                                             size_t index) {
  DriveTarget.Update();
  const double drive = DriveTarget.GetDrive();
  const double reduction = DriveTarget.GetDriveReduction();
  return {lerp(inLeft,
               ApplyAnalogDistortion(m_BufferLeft[index], drive, reduction),
               m_DistortionAmount),
          lerp(inRight,
               ApplyAnalogDistortion(m_BufferRight[index], drive, reduction),
               m_DistortionAmount)};
}

AnalogChannel AnalogMode::ApplyPost(const double inL, const double inR) {
  return {
      m_SmoothFilter.ApplyLeft(inL),
      m_SmoothFilter.ApplyRight(inR),
  };
}
void AnalogMode::Resize(const size_t maxSize) {
  m_BufferLeft.resize(maxSize);
  m_BufferRight.resize(maxSize);
}

void AnalogMode::SetupFilter(const double sR) {
  m_AnalogFilter.SetSampleRate(sR);
  m_AnalogFilter.SetFilterType(Filter::Type::LowPass);
  // the AnalogFilter needs to cut to avoid Aliasing ;)
  m_AnalogFilter.CalculateCoefficients(48.0, 300, 0.707);

  m_SmoothFilter.SetSampleRate(sR);
  m_SmoothFilter.SetFilterType(Filter::Type::HighShelf);
  m_SmoothFilter.CalculateCoefficients(m_Gain, 18000, 0.45);
}

void AnalogMode::CalculateWarmEffect(const float value) {
  m_Gain = lerp(-0.05, -1.5, value);
  m_SmoothFilter.CalculateCoefficients(m_Gain, 18000, 0.45);
  m_DistortionAmount = lerp(0.01, 0.2f, value);
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
    auto [Left, Right] = ApplyPreDistortion(buf.Left, buf.Right, i);
    m_BufferLeft[i] = Left;
    m_BufferRight[i] = Right;
  }
}

} // namespace VSTZ
