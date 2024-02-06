#include "TargetDrive.h"

#include "../Utils/Audio.h"

#include <algorithm>
#include <cmath>

namespace VSTZ {
void TargetDrive::CalculateDrive(const float *leftBuffer,
                                 const float *rightBuffer, size_t size) {
  float avg = 0;
  for (size_t i = 0; i < size; i++) {
    const float val = std::abs((leftBuffer[i] + rightBuffer[i]) * 0.5f);
    avg += val;
  }
  m_SmoothedGain = m_Alpha * (avg / size) + (1 - m_Alpha) * m_SmoothedGain;
  ApplyDrive();
}

void TargetDrive::CalculateDrive(const double *leftBuffer,
                                 const double *rightBuffer, size_t size) {
  double avg = 0;
  for (size_t i = 0; i < size; i++) {
    const double val = std::abs((leftBuffer[i] + rightBuffer[i]) * 0.5);
    avg += val;
  }
  m_SmoothedGain = m_Alpha * (avg / size) + (1 - m_Alpha) * m_SmoothedGain;
  ApplyDrive();
}

void TargetDrive::ApplyDrive() {

  const double dB = AudioUtils::GainToDecibels(m_SmoothedGain);
  const double gain = std::clamp(0.0 - dB, -MaxDrive, MaxDrive);
  m_CurrentGain = gain;
  m_DriveGain = AudioUtils::DecibelToGain(gain);
  m_DriveGainReduction = AudioUtils::DecibelToGain(-gain);
}
} // namespace VSTZ