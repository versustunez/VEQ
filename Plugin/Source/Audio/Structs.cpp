#include "Structs.h"

#include "Utils/Audio.h"

namespace VSTZ {
#pragma region Channel
double Channel::GetMax() const { return std::max(Left, Right); }
double Channel::GetMin() const { return std::min(Left, Right); }
double Channel::GetMaxAbs() const {
  return std::max(std::abs(Left), std::abs(Right));
}
Channel Channel::operator*=(const Channel &other) {
  Left *= other.Left;
  Right *= other.Right;
  return *this;
}
Channel Channel::operator+=(const Channel &other) {
  Left += other.Left;
  Right += other.Right;
  return *this;
}
Channel Channel::operator/=(const float other) {
  Left /= other;
  Right /= other;
  return *this;
}
Channel Channel::operator*=(double other) {
  Left *= other;
  Right *= other;
  return *this;
}
Channel Channel::operator+=(double other) {
  Left += other;
  Right += other;
  return *this;
}
Channel Channel::operator-(const Channel &other) const {
  return {Left - other.Left, Right - other.Right};
}
Channel Channel::operator*(const Channel &other) const {
  return {Left + other.Left, Right + other.Right};
}
Channel Channel::operator*(const double &other) const {
  return {Left * other, Right * other};
}
Channel Channel::toGain() const {
  return {
      AudioUtils::DecibelToGain(Left),
      AudioUtils::DecibelToGain(Right),
  };
}
Channel Channel::toDecibels() const {
  return {
      AudioUtils::GainToDecibels(Left),
      AudioUtils::GainToDecibels(Right),
  };
}
constexpr double sqrt2 = 1.4142135623730951;
Channel Channel::midSide() const {
  return {
      Left + Right / sqrt2,
      Left - Right / sqrt2,
  };
}

#pragma endregion

void Smoother::setInitial(double value) { Previous = value; }
double Smoother::Get(double in) {
  Previous += Coeff * (in - Previous);
  return Previous;
}
void Smoother::SetSampleRate(double sampleRate) {
  Coeff = exp(-1.0 / (0.04 * sampleRate));
}

double DCFilter::Process(double in) {
  const double y = in - m_In + R * m_Out;
  m_In = in;
  m_Out = y;
  return y;
}
void DCFilter::SetSampleRate(double sampleRate) {
  constexpr double cutoff = 10.0; // try 10–20 Hz
  constexpr double PI = 3.14159265358979323846;
  R = exp(-2.0 * PI * cutoff / sampleRate);
}

} // namespace VSTZ