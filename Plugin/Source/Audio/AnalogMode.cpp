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
  const double distortedSignal =
      std::atan(in * driveAmount) * driveAmountReduction;
  return std::clamp(distortedSignal, -clipThreshold, clipThreshold);
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
  return {ApplySlewLimiter(
              lerp(inL, ApplyAnalogDistortion(inL), m_DistortionAmount),
              m_LastValueLeft, m_AnalogSlew),
          ApplySlewLimiter(
              lerp(inR, ApplyAnalogDistortion(inR), m_DistortionAmount),
              m_LastValueRight, m_AnalogSlew)};
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