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

static double ApplyFirstOrderLowPass(const double in, double &lastValue,
                                     const double alpha) {
  lastValue = alpha * lastValue + (1.0 - alpha) * in;
  return lastValue;
}

AnalogChannel AnalogMode::ApplyPreDistortion(const double inLeft,
                                             const double inRight) {

  return {lerp(inLeft,
               ApplyAnalogDistortion(m_AnalogFilter.ApplyLeft(inLeft),
                                     DriveTarget.GetDrive(),
                                     DriveTarget.GetDriveReduction()),
               m_DistortionAmount),
          lerp(inRight,
               ApplyAnalogDistortion(m_AnalogFilter.ApplyRight(inRight),
                                     DriveTarget.GetDrive(),
                                     DriveTarget.GetDriveReduction()),
               m_DistortionAmount)};
}

AnalogChannel AnalogMode::ApplyPostDistortion(const double inL,
                                              const double inR) {
  return {lerp(inL, ApplyFirstOrderLowPass(inL, m_LastValueLeft, m_Alpha),
               m_AlphaMix),
          lerp(inR, ApplyFirstOrderLowPass(inR, m_LastValueRight, m_Alpha),
               m_AlphaMix)};
}

void AnalogMode::SetupFilter(const double sR) {
  m_AnalogFilter.SetSampleRate(sR);
  m_AnalogFilter.SetFilterType(Filter::Type::LowPass);
  // the AnalogFilter needs to cut to avoid Aliasing ;)
  m_AnalogFilter.CalculateCoefficients(48.0, 500, 0.707);

  // we have to see :)
  m_Alpha = std::exp(-2.0 * juce::MathConstants<double>::pi * 10000 / sR);
}

void AnalogMode::CalculateWarmEffect(float value) {
  m_AlphaMix = lerp(0.1, 0.9, value);
  m_DistortionAmount = lerp(0.01, 0.15f, value);
}

} // namespace VSTZ