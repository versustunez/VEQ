#pragma once


#include <cstddef>

namespace VSTZ {
struct TargetDrive {
  static constexpr double MaxDrive{50};
  void CalculateDrive(const float *leftBuffer, const float *rightBuffer,
                                 size_t size);
  void CalculateDrive(const double *leftBuffer, const double *rightBuffer,
                                 size_t size);
  void NextDrive();

  double GetDriveGain() const { return m_CurrentGain; }
  double GetDrive() const { return m_DriveGain; }
  double GetDriveReduction() const { return m_DriveGainReduction; }

private:
  void ApplyDrive();
  // we calculate them by input signals! but also limit them by ~30dB if the
  // if signal is < -70 we dont increase it or lower it ;)
  // the drive also needs to fast lowering and slow rising to
  double m_DriveGain{1.0};
  double m_DriveGainReduction{1.0};
  double m_SmoothedGain{1.0};
  double m_DriveTarget{0.0};
  double m_CurrentGain{0};

  constexpr static size_t BufferSize{256};
  float m_RMSBuffer[BufferSize]{0};
  size_t m_RMSIndex{0};
  size_t m_StepsRequired{0};
  size_t m_Steps{0};
  double m_ReductionRate{0.0};
};
} // namespace VSTZ