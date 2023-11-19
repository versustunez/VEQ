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
  constexpr float slope_dB_per_octave = -4.5f; // this matches PRO-Q3 by default :)
  constexpr float lowestFrequency = 20.0f;
  if (!m_FFT->IsDirty())
    return;
  float lowestFrequencyLog = std::log2(lowestFrequency);
  m_FFT->ApplyFFT();
  const auto *fftData = m_FFT->GetFFTData();
  constexpr auto fftSize = FFT::GetFFTSize();
  const float offset =
      juce::Decibels::gainToDecibels(static_cast<float>(fftSize));
  const auto sampleRate = Core::Config::get().sampleRate;
  constexpr float infinity = -70.0f;

  for (int i = 0; i < fftSize; ++i) {
    const float freq = i * sampleRate / fftSize;
    float bindB = juce::Decibels::gainToDecibels(fftData[i], infinity) - offset;
    if (freq > lowestFrequency) {
      const float logFreq = std::log2(freq);
      bindB = bindB - slope_dB_per_octave * (logFreq - lowestFrequencyLog);
    }
    const float binData = juce::jmap(bindB, infinity, 6.0f, 0.0f, 1.0f);
    if (m_FFTData[i] < binData) {
      m_FFTData[i] = binData;
    } else {
      m_FFTData[i] *= 0.95f; // maybe the falloff speed needs to be configured :)
    }
  }
}
} // namespace VSTZ::Editor