#pragma once
namespace VSTZ {

class Filter {
public:
  enum class Type { LowShelf, HighShelf, Peak, NO };

  void SetFilterType(Type type);
  void SetSampleRate(float sampleRate);
  void Reset();
  float ApplyLeft(float in);
  float ApplyRight(float in);
  bool IsBypassed() const { return m_Type == Type::NO; }

  void CalculateCoefficients(float gain, float frequency, float q = 0.0);

  double GetMagnitudeForFrequency(double frequency, double sampleRate);

protected:
  Type m_Type{Type::NO};
  float m_SampleRate{44100.0f};

  float m_B0{0}, m_B1{0}, m_B2{0};
  float m_A0{0}, m_A1{0}, m_A2{0};

  float m_xL1{0}, m_xL2{0};
  float m_yL1{0}, m_yL2{0};

  float m_xR1{0}, m_xR2{0};
  float m_yR1{0}, m_yR2{0};
};

} // namespace VSTZ
