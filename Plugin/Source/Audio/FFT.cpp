#include "FFT.h"

namespace VSTZ {
FFT::FFT()
    : m_ForwardFFT(FFTOrder),
      m_Window(FFTSize, juce::dsp::WindowingFunction<float>::hann, true) {}

void FFT::PushSample(const float sample) {
  if (m_FifoIndex == FFTSize) {
    if (!m_NextFFTBlockReady) {
      juce::zeromem(m_FFTData, sizeof(m_FFTData));
      memcpy(m_FFTData, m_Fifo, sizeof(m_Fifo));
      m_NextFFTBlockReady = true;
    }
    m_FifoIndex = 0;
  }
  m_Fifo[m_FifoIndex++] = sample;
}

void FFT::ApplyFFT() {
  if (!m_NextFFTBlockReady) {
    return;
  }
  m_Window.multiplyWithWindowingTable(m_FFTData, FFTSize);
  m_ForwardFFT.performFrequencyOnlyForwardTransform(m_FFTData);
  // juce::FloatVectorOperations::multiply (m_FFTData, 2.0f, FFTSize);
  m_NextFFTBlockReady = false;
}
} // namespace VSTZ