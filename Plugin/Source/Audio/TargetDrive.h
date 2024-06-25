#pragma once

#include <cstddef>

namespace VSTZ {
struct TargetDrive {
  static constexpr double MaxDrive{50};
  void CalculateDrive(const float *leftBuffer, const float *rightBuffer,
                      size_t size);
  void CalculateDrive(const double *leftBuffer, const double *rightBuffer,
                      size_t size);

  double GetDriveGain() const { return m_CurrentDriveDecibels; }
  double GetDrive() const { return m_CurrentTarget.Gain; }
  double GetDriveReduction() const { return m_CurrentTarget.Reduction; }

  void Update();

private:
  void ApplyDrive();
  struct Target {
    double Gain{1.0};
    double Reduction{1.0};
  };

  double m_SmoothedGain{1.0};
  double m_CurrentDriveDecibels{0};
  Target m_CurrentTarget{};
  Target m_TargetedTarget{};
  double m_SmoothingConstant{0.2};
  double m_GainSmoothingConstant{0.5};
};
} // namespace VSTZ