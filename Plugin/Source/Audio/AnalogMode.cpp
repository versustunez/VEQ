#include "AnalogMode.h"

#include <algorithm>
#include <cmath>

namespace VSTZ {
static double driveAmount = std::pow(10.0f, 6.0f / 20.0f);
static double driveAmountReduction = std::pow(10.0f, -6.0f / 20.0f);
constexpr double clipThreshold = 1.0f;

static double lerp(const double a, const double b, const double alpha) {
  return a + alpha * (b - a);
}

static double ApplyAnalogDistortion(const double in) {
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

static double ApplySlewLimiter(const double in, double &lastValue,
                               const double slewRate = 0.75) {
  double diff = in - lastValue;
  diff = std::clamp(diff, -slewRate, slewRate);
  lastValue = lastValue + diff;
  return lastValue;
}

AnalogChannel AnalogMode::ApplyPreDistortion(const double inLeft,
                                             const double inRight) {

  return {lerp(inLeft, ApplyAnalogDistortion(m_AnalogFilter.ApplyLeft(inLeft)),
               m_DistortionAmount),
          lerp(inRight,
               ApplyAnalogDistortion(m_AnalogFilter.ApplyRight(inRight)),
               m_DistortionAmount)};
}

AnalogChannel AnalogMode::ApplyPostDistortion(const double inL,
                                              const double inR) {
  return {ApplySlewLimiter(std::clamp(inL, -1.0, 1.0), m_LastValueLeft,
                           m_AnalogSlew),
          ApplySlewLimiter(std::clamp(inR, -1.0, 1.0), m_LastValueRight,
                           m_AnalogSlew)};
}

void AnalogMode::SetupFilter(const double sR) {
  m_AnalogFilter.SetSampleRate(sR);
  m_AnalogFilter.SetFilterType(Filter::Type::LowPass);
  // the AnalogFilter needs to cut to avoid Aliasing ;)
  m_AnalogFilter.CalculateCoefficients(48.0, 500, 0.707);
}

void AnalogMode::CalculateWarmEffect(float value) {
  m_AnalogSlew = lerp(0.95, 0.5, value);
  m_DistortionAmount = lerp(0.01, 0.1f, value);
}
void AnalogMode::ResetSlew(float left, float right) {
  m_LastValueLeft = left;
  m_LastValueRight = right;
}

} // namespace VSTZ