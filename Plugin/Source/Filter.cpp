#include "Filter.h"

#include <JuceHeader.h>
#include <cmath>
#include <complex>

namespace VSTZ {

constexpr double twoPi = 2.0 * 3.14159265359;

void Filter::Reset() {
  m_xL1 = m_xL2 = 0.0f;
  m_yL1 = m_yL2 = 0.0f;

  m_xR1 = m_xR2 = 0.0f;
  m_yR1 = m_yR2 = 0.0f;

  m_B0 = m_B1 = m_B2 = 0.0f;
  m_A0 = m_A1 = m_A2 = 0.0f;
}

void Filter::SetFilterType(Filter::Type type) { m_Type = type; }
void Filter::SetSampleRate(double sampleRate) { m_SampleRate = sampleRate; }

double Filter::ApplyLeft(double in) {
  double yL0 =
      (m_B0 * in + m_B1 * m_xL1 + m_B2 * m_xL2 - m_A1 * m_yL1 - m_A2 * m_yL2);

  m_xL2 = m_xL1;
  m_xL1 = in;

  m_yL2 = m_yL1;
  m_yL1 = yL0;

  return yL0;
}
double Filter::ApplyRight(double in) {
  double yR0 =
      (m_B0 * in + m_B1 * m_xR1 + m_B2 * m_xR2 - m_A1 * m_yR1 - m_A2 * m_yR2);

  m_xR2 = m_xR1;
  m_xR1 = in;

  m_yR2 = m_yR1;
  m_yR1 = yR0;

  return yR0;
}

void Filter::CalculateCoefficients(double gain, double frequency, double q) {
  double A = pow(10.0f, (gain / 20.0f));
  double omega = (float)twoPi * frequency / m_SampleRate;
  double tsin = sin(omega);
  double tcos = cos(omega);

  switch (m_Type) {
  case Type::LowShelf: {
    double beta = sqrt(A + A) / q;
    m_A0 = (A + 1.0) + (A - 1.0) * tcos + beta * tsin;
    m_A1 = (-2.0 * ((A - 1.0) + (A + 1.0) * tcos)) / m_A0;
    m_A2 = ((A + 1.0) + (A - 1.0) * tcos - beta * tsin) / m_A0;

    m_B0 = (A * ((A + 1.0) - (A - 1.0) * tcos + beta * tsin)) / m_A0;
    m_B1 = (2.0 * A * ((A - 1.0) - (A + 1.0) * tcos)) / m_A0;
    m_B2 = (A * ((A + 1.0) - (A - 1.0) * tcos - beta * tsin)) / m_A0;
  } break;
  case Type::HighShelf: {
    double beta = sqrt(A + A) / q;
    m_A0 = (A + 1.0) - (A - 1.0) * tcos + beta * tsin;
    m_A1 = (2.0 * ((A - 1.0) - (A + 1.0) * tcos)) / m_A0;
    m_A2 = ((A + 1.0) - (A - 1.0) * tcos - beta * tsin) / m_A0;

    m_B0 = (A * ((A + 1.0) + (A - 1.0) * tcos + beta * tsin)) / m_A0;
    m_B1 = (-2.0 * A * ((A - 1.0) + (A + 1.0) * tcos)) / m_A0;
    m_B2 = (A * ((A + 1.0) + (A - 1.0) * tcos - beta * tsin)) / m_A0;
  } break;
  case Type::Peak: {
    double alpha = tsin / (2.0 * q);
    m_A0 = (1.0f + alpha / A);
    m_A1 = (-2.0f * tcos) / m_A0;
    m_A2 = (1.0f - alpha / A) / m_A0;

    m_B0 = (1.0f + alpha * A) / m_A0;
    m_B1 = (-2.0f * tcos) / m_A0;
    m_B2 = (1.0f - alpha * A) / m_A0;
  } break;
  case Type::LowPass: {
    double alpha = tsin / (2.0 * q);
    double denominator = 1.0 + alpha;

    m_B0 = (1.0 - tcos) / 2.0 / denominator;
    m_B1 = (1.0 - tcos) / denominator;
    m_B2 = (1.0 - tcos) / 2.0 / denominator;
    m_A0 = 1.0;
    m_A1 = -2.0 * tcos / denominator;
    m_A2 = (1.0 - alpha) / denominator;
  } break;
  case Type::HighPass: {
    double alpha = tsin / (2.0 * q);
    double denominator = 1.0 + alpha;

    m_B0 = (1.0 + tcos) / 2.0 / denominator;
    m_B1 = -1.0 * (1.0 + tcos) / denominator;
    m_B2 = (1.0 + tcos) / 2.0 / denominator;
    m_A0 = 1.0;
    m_A1 = -2.0 * tcos / denominator;
    m_A2 = (1.0 - alpha) / denominator;
  } break;
  case Type::NO:
  default: Reset();
  }
}

double Filter::GetMagnitudeForFrequency(double frequency, double) const {

  double omega = twoPi * frequency / m_SampleRate;

  std::complex<double> numerator =
      m_B0 + m_B1 * std::exp(std::complex<double>(0, -omega)) +
      m_B2 * std::exp(std::complex<double>(0, -2.0 * omega));
  std::complex<double> denominator =
      1.0 + m_A1 * std::exp(std::complex<double>(0, -omega)) +
      m_A2 * std::exp(std::complex<double>(0, -2.0 * omega));

  return std::abs(numerator / denominator);
}

} // namespace VSTZ
