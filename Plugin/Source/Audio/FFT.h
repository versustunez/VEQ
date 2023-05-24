#pragma once

#include <JuceHeader.h>

namespace VSTZ {

class FFT {
public:
  FFT();
  void PushSample(float sample);
  float *GetFFTData() { return m_FFTData; }
  constexpr static int GetFFTSize() { return FFTSize; }
  void ClearDirtyMark() { m_NextFFTBlockReady = false; }
  [[nodiscard]] bool IsDirty() const { return m_NextFFTBlockReady; }
  void ApplyFFT();

protected:
  constexpr static int FFTOrder = 11;
  constexpr static int FFTSize = 1 << FFTOrder;
  juce::dsp::FFT m_ForwardFFT;
  juce::dsp::WindowingFunction<float> m_Window;

  float m_Fifo[FFTSize]{};
  float m_FFTData[2 * FFTSize]{};
  int m_FifoIndex{0};
  bool m_NextFFTBlockReady{false};
};

} // namespace VSTZ
