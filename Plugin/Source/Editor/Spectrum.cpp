#include "Spectrum.h"

#include "Core/Config.h"

namespace VSTZ::Editor {
static float indexToX(float index, float minFreq, float sampleRate, int size) {
  const auto freq = (sampleRate * index) / size;
  return (freq > 0.01f) ? std::log(freq / minFreq) / std::log(2.0f) : 0.0f;
}

void Spectrum::paint(juce::Graphics &g) {
  auto sampleRate = Core::Config::get().sampleRate;
  auto fftSize = FFT::GetFFTSize();

  PrepareFFT();

  juce::Path p;
  p.startNewSubPath(0, getHeight());
  p.preallocateSpace(10 + 3 * fftSize);
  auto bounds = getLocalBounds().toFloat();
  const auto factor = bounds.getWidth() / 10.0f;
  p.startNewSubPath(0, getHeight());
  for (int i = 0; i < fftSize; ++i) {
    p.lineTo(bounds.getX() +
                 factor * indexToX(float(i), 20, sampleRate, fftSize),
             (m_FFTData[i] * -1.0f + 1.0f) * bounds.getHeight());
  }
  p.lineTo(bounds.getWidth(), getHeight());
  p.closeSubPath();

  g.setColour(m_SpectrumColor);
  g.fillPath(p);
}

void Spectrum::PrepareFFT() {
  if (!m_FFT->IsDirty())
    return;
  m_FFT->ApplyFFT();
  auto *fftData = m_FFT->GetFFTData();
  auto fftSize = FFT::GetFFTSize();
  const float offset = juce::Decibels::gainToDecibels((float)fftSize);

  const float infinity = -70.0f;

  for (int i = 0; i < fftSize; ++i) {
    float binData = juce::jmap(
        juce::Decibels::gainToDecibels(fftData[i], infinity) - offset, infinity,
        6.0f, 0.0f, 1.0f);
    if (m_FFTData[i] < binData) {
      m_FFTData[i] = binData;
    } else {
      m_FFTData[i] *= 0.9f;
    }
  }
}
} // namespace VSTZ::Editor