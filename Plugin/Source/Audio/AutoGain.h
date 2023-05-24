#pragma once

namespace VSTZ {
class AutoGain {
public:
  void SetSampleRate(double sampleRate) {
    m_SampleRate = sampleRate;
    CalculateSmoothingFactor();
  }
  double GetGain() { return m_Gain; }
  double GetPreviousGain() { return m_PreviousGain; }

  template <typename T> void CalculateInputGain(juce::AudioBuffer<T> &buffer) {
    m_InputGain = 0;
    for (int i = 0; i < buffer.getNumChannels(); ++i)
      m_InputGain += buffer.getRMSLevel(i, 0, buffer.getNumSamples());
  }

  template <typename T> void CalculateOutputGain(juce::AudioBuffer<T> &buffer) {
    double outputGain = 0.0;
    for (int i = 0; i < buffer.getNumChannels(); ++i)
      outputGain += buffer.getRMSLevel(i, 0, buffer.getNumSamples());

    double makeup = (outputGain > 0.0 && m_InputGain > 0.0)
                        ? m_InputGain / outputGain
                        : 1.0;
    m_PreviousGain = m_Gain;
    m_Gain = m_SmoothnessFactor * m_Gain + (1.0f - m_SmoothnessFactor) * makeup;
  }

protected:
  void CalculateSmoothingFactor() {
    constexpr double desiredTimeConstant = 0.1 / 1000.0;
    double numSamples = desiredTimeConstant * m_SampleRate;
    m_SmoothnessFactor = std::exp(-1.0 / numSamples);
  }

protected:
  double m_SampleRate{44100.0};
  double m_Gain{1.0};
  double m_PreviousGain{1.0};
  double m_SmoothnessFactor{0.0};
  double m_InputGain{0.0};
};
} // namespace VSTZ
