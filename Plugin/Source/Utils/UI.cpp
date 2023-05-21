#include "UI.h"

namespace VSTZ::Utils {
float UI::GetDecibelScale(const ScaleData &data) {
  if (data.Type->getInt() == 0)
    return 6.0f;

  double parameterValue = std::abs(data.Gain->getValue());
  if (parameterValue < 5.0f)
    return 6.0f;
  else if (parameterValue < 10.0f)
    return 12.0f;
  else if (parameterValue < 20.0f)
    return 24.0f;
  return 30.0f;
}

float UI::GetDecibelScaleForArray(const ScaleData *data, int size) {
  float scale = 6.0f;
  for (int i = 0; i < size; i++) {
    scale = std::max(scale, GetDecibelScale(data[i]));
  }
  return scale;
}
} // namespace VSTZ::Utils