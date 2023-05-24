#pragma once

#include "TypeDefs.h"

#include <JuceHeader.h>

namespace VSTZ {
class AutoGain {
public:
  AutoGain();
  void SetSampleRate(double sampleRate) { m_SampleRate = sampleRate; }
  double GetGain() { return m_Gain; }

  template <typename T> void CalculateInputGain(juce::AudioBuffer<T> &buffer) {
    auto *leftData = buffer.getReadPointer(0);
    auto *rightData = buffer.getReadPointer(1);
    for (int j = 0; j < buffer.getNumSamples(); ++j) {
      m_In.Right[m_In.Index] = rightData[j];
      m_In.Left[m_In.Index++] = leftData[j];
      if (m_In.Index >= BufferSize) {
        m_In.Index = 0;
        m_InputGain = CalculateRMS(m_In);
      }
    }
  }

  template <typename T> void CalculateOutputGain(juce::AudioBuffer<T> &buffer) {
    auto *leftData = buffer.getWritePointer(0);
    auto *rightData = buffer.getWritePointer(1);
    for (int j = 0; j < buffer.getNumSamples(); ++j) {
      m_Out.Left[m_Out.Index] = leftData[j];
      m_Out.Right[m_Out.Index++] = rightData[j];
      if (m_Out.Index >= BufferSize) {
        m_Out.Index = 0;
        CalculateMakeupGain();
      }
    }
  }

protected:
  constexpr static size_t BufferSize{256};
  struct RingBuffer {
    double Left[BufferSize]{};
    double Right[BufferSize]{};
    uint32_t Index = 0;
  };

  void CalculateSmoothingFactor();
  void CalculateMakeupGain();
  static double CalculateRMS(RingBuffer &buffer) ;

protected:
  RingBuffer m_In, m_Out;
  double m_SampleRate{44100.0};
  double m_Gain{1.0};
  double m_SmoothnessFactor{0.0};
  double m_InputGain{0.0};
};
} // namespace VSTZ
