#include "TargetDrive.h"

#include "../Utils/Audio.h"

#include <algorithm>
#include <cmath>

namespace VSTZ {
void TargetDrive::CalculateDrive(const float *leftBuffer,
                                 const float *rightBuffer, size_t size) {
  for (size_t i = 0; i < size; i++) {
    m_RMSBuffer[m_RMSIndex++] = (leftBuffer[i] + rightBuffer[i]) * 0.5f;
    if (m_RMSIndex == BufferSize) {
      ApplyDrive();
    }
  }
}

void TargetDrive::CalculateDrive(const double *leftBuffer,
                                 const double *rightBuffer, size_t size) {
  for (size_t i = 0; i < size; i++) {
    m_RMSBuffer[m_RMSIndex++] =
        static_cast<float>((leftBuffer[i] + rightBuffer[i]) * 0.5);
    if (m_RMSIndex == BufferSize) {
      ApplyDrive();
    }
  }
}

static constexpr size_t m_ReductionRateSlow = 100;
static constexpr size_t m_ReductionRateFast = 2;

static constexpr double m_AlphaOne = 1.0 / m_ReductionRateSlow;
static constexpr double m_AlphaTwo = 1.0 / 2.0;

void TargetDrive::ApplyDrive() {
  m_RMSIndex = 0;
  double sumSquared = 0.0;
  for (const float i : m_RMSBuffer) {
    sumSquared += i * i;
  }
  // we have a silent signal.. dont boost!
  if (sumSquared < 0.001) {
    return;
  }
  m_DriveTarget = std::sqrt(sumSquared / BufferSize);
  m_ReductionRate = (m_DriveTarget < m_SmoothedGain) ? m_AlphaTwo : m_AlphaOne;
  m_StepsRequired = (m_DriveTarget < m_SmoothedGain) ? m_ReductionRateFast
                                                     : m_ReductionRateSlow;
  m_Steps = 0;
}

void TargetDrive::NextDrive() {
  if (m_Steps == m_StepsRequired)
    return;

  const double alpha = m_ReductionRate;
  m_SmoothedGain = alpha * m_DriveTarget + (1 - alpha) * m_SmoothedGain;

  const double gain = std::clamp(
      0.0 - AudioUtils::GainToDecibels(m_SmoothedGain), -MaxDrive, MaxDrive);
  m_CurrentGain = gain;
  m_DriveGain = AudioUtils::DecibelToGain(gain);
  m_DriveGainReduction = AudioUtils::DecibelToGain(-gain);
}
} // namespace VSTZ