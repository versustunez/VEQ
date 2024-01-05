#include "Spectrum.h"

#include "Core/Config.h"

namespace VSTZ::Editor {

constexpr float slopeDecibelsPerOctave = -3.0f;
constexpr float lowestFrequency = 20.0f;
static const float lowestFrequencyLog = std::log2(1200.0f);

static float indexToX(const float index, const float sampleRate,
                      const float size) {
  const auto freq = (sampleRate * index) / size;
  return (freq > 0.01f) ? std::log2((freq + lowestFrequency) / lowestFrequency)
                        : 0.0f;
}

void Spectrum::createFFTPath(const double sampleRate, const int fftSize) {
  const auto bounds = getLocalBounds().toFloat();
  const auto sR = static_cast<float>(sampleRate);
  const auto fftSizeF = static_cast<float>(fftSize);

  auto &p = m_PathTmp;
  p.clear();
  p.preallocateSpace(10 + 3 * fftSize);

  const auto factor = bounds.getWidth() / 10.0f;
  p.startNewSubPath(factor * indexToX(0, sR, fftSizeF), bounds.getHeight());
  for (int i = 1; i < fftSize; ++i) {
    const float idxX = indexToX(static_cast<float>(i), sR, fftSizeF);
    const float x = bounds.getX() + factor * idxX;
    const float y = std::lerp(bounds.getBottom(), bounds.getY(), m_FFTData[i]);
    p.lineTo(x, y);
  }
  p.lineTo(bounds.getWidth(), bounds.getHeight());
  p.closeSubPath();
  m_Path = m_PathTmp.createPathWithRoundedCorners(10.0f);
}
void Spectrum::paint(juce::Graphics &g) {
  if (PrepareFFT()) {
    const auto sampleRate = Core::Config::get().sampleRate;
    constexpr auto fftSize = FFT::GetFFTSize();
    createFFTPath(sampleRate, fftSize);
  }

  g.setColour(m_SpectrumColor.withAlpha(0.2f));
  g.fillPath(m_Path);

  g.setColour(m_SpectrumColor);
  g.strokePath(m_Path, juce::PathStrokeType(1.0f));
}

void Spectrum::resized() {
  const double sampleRate = Core::Config::get().sampleRate;
  constexpr int fftSize = FFT::GetFFTSize();
  createFFTPath(sampleRate, fftSize);
}

bool Spectrum::PrepareFFT() {
  if (!m_FFT->IsDirty())
    return false;
  m_FFT->ApplyFFT();

  const auto *fftData = m_FFT->GetFFTData();
  constexpr auto fftSize = FFT::GetFFTSize();
  const float offset =
      juce::Decibels::gainToDecibels(static_cast<float>(fftSize));
  const auto sampleRate = static_cast<float>(Core::Config::get().sampleRate);
  constexpr float infinity = -70.0f;

  for (int i = 1; i < fftSize; ++i) {
    const float freq = static_cast<float>(i) * sampleRate / fftSize;
    float bindB = juce::Decibels::gainToDecibels(fftData[i], infinity) - offset;
    if (freq > lowestFrequency) {
      const float logFreq = std::log2(freq);
      bindB = bindB - slopeDecibelsPerOctave * (logFreq - lowestFrequencyLog);
    }
    const float binData = juce::jmap(bindB, infinity, 0.0f, 0.0f, 1.0f);
    if (m_FFTData[i] < binData) {
      m_FFTData[i] = binData;
    } else {
      m_FFTData[i] *=
          0.95f; // maybe the falloff speed needs to be configured :)
    }
  }
  return true;
}
} // namespace VSTZ::Editor
