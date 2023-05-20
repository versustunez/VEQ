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
void Filter::SetSampleRate(float sampleRate) { m_SampleRate = sampleRate; }

float Filter::ApplyLeft(float in) {
  float yL0 =
      (m_B0 * in + m_B1 * m_xL1 + m_B2 * m_xL2 - m_A1 * m_yL1 - m_A2 * m_yL2);

  m_xL2 = m_xL1;
  m_xL1 = in;

  m_yL2 = m_yL1;
  m_yL1 = yL0;

  return yL0;
}
float Filter::ApplyRight(float in) {
  float yR0 =
      (m_B0 * in + m_B1 * m_xR1 + m_B2 * m_xR2 - m_A1 * m_yR1 - m_A2 * m_yR2);

  m_xR2 = m_xR1;
  m_xR1 = in;

  m_yR2 = m_yR1;
  m_yR1 = yR0;

  return yR0;
}

void Filter::CalculateCoefficients(float gain, float frequency, float q) {
  float A = powf(10.0f, (gain / 20.0f));
  float omega = (float)twoPi * frequency / m_SampleRate;
  float tsin = sinf(omega);
  float tcos = cosf(omega);
  float beta = sqrtf(A + A);

  switch (m_Type) {
  case Type::LowShelf: {
    m_A0 = (A + 1.0f) + (A - 1.0f) * tcos + beta * tsin;
    m_A1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * tcos)) / m_A0;
    m_A2 = ((A + 1.0f) + (A - 1.0f) * tcos - beta * tsin) / m_A0;

    m_B0 = (A * ((A + 1.0f) - (A - 1.0f) * tcos + beta * tsin)) / m_A0;
    m_B1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * tcos)) / m_A0;
    m_B2 = (A * ((A + 1.0f) - (A - 1.0f) * tcos - beta * tsin)) / m_A0;
  } break;
  case Type::HighShelf: {
    m_A0 = (A + 1.0f) - (A - 1.0f) * tcos + beta * tsin;
    m_A1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * tcos)) / m_A0;
    m_A2 = ((A + 1.0f) - (A - 1.0f) * tcos - beta * tsin) / m_A0;

    m_B0 = (A * ((A + 1.0f) + (A - 1.0f) * tcos + beta * tsin)) / m_A0;
    m_B1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * tcos)) / m_A0;
    m_B2 = (A * ((A + 1.0f) + (A - 1.0f) * tcos - beta * tsin)) / m_A0;
  } break;
  case Type::Peak: {
    float alpha = tsin / (2.0f * q);

    m_A0 = (1.0f + alpha / A);
    m_A1 = (-2.0f * tcos) / m_A0;
    m_A2 = (1.0f - alpha / A) / m_A0;

    m_B0 = (1.0f + alpha * A) / m_A0;
    m_B1 = (-2.0f * tcos) / m_A0;
    m_B2 = (1.0f - alpha * A) / m_A0;
  } break;
  default: Reset();
  }
}
double Filter::GetMagnitudeForFrequency(double frequency, double) {

  float omega = juce::MathConstants<float>::twoPi * frequency / m_SampleRate;

  std::complex<float> numerator =
      m_B0 + m_B1 * std::exp(std::complex<float>(0, -omega)) +
      m_B2 * std::exp(std::complex<float>(0, -2.0f * omega));
  std::complex<float> denominator =
      1.0f + m_A1 * std::exp(std::complex<float>(0, -omega)) +
      m_A2 * std::exp(std::complex<float>(0, -2.0f * omega));

  return std::abs(numerator / denominator);
}

} // namespace VSTZ