#pragma once

#include <algorithm>

namespace VSTZ {
struct Channel {
  double Left{0.0};
  double Right{0.0};
  double GetMax() const;
  double GetMin() const;
  double GetMaxAbs() const;
  Channel operator*=(const Channel &other);
  Channel operator+=(const Channel &other);
  Channel operator/=(float other);
  Channel operator*=(double other);
  Channel operator+=(double other);
  Channel operator-(const Channel &other) const;
  Channel operator*(const Channel &other) const;
  Channel operator*(const double &other) const;
  [[nodiscard]] Channel toGain() const;
  [[nodiscard]] Channel toDecibels() const;
  [[nodiscard]] Channel midSide() const;
};

struct InOut {
  double InL{0}, OutL{0};
  double InR{0}, OutR{0};
};

struct Smoother {
  double Previous{0};
  double Coeff{0};

  void setInitial(double value);
  double Get(double in);
  void SetSampleRate(double sampleRate);
};

struct DCFilter {

  double Process(double in);
  void SetSampleRate(double sampleRate);

private:
  double m_In = 0.0;
  double m_Out = 0.0;
  double R = 0.9995;
};

} // namespace VSTZ