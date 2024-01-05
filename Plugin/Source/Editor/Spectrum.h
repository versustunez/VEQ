#pragma once

#include "Audio/FFT.h"
#include "GUI/Components/VComponent.h"

#include <TypeDefs.h>

namespace VSTZ::Editor {
class Spectrum final : public GUI::VComponent {
public:
  explicit Spectrum(const InstanceID id, FFT *usingFFT) : m_ID(id), m_FFT(usingFFT) {
  }
  void createFFTPath(double sampleRate, int fftSize);
  void paint(juce::Graphics &g) override;
  void resized() override;

  void SetColor(const float red, const float green, const float blue, const float alpha = 1.0f) {
    m_SpectrumColor = juce::Colour::fromFloatRGBA(red, green, blue, alpha);
  }

  void SetColor(juce::Colour color) {
    m_SpectrumColor = color;
  } 

protected:
  bool PrepareFFT();

protected:
  InstanceID m_ID;
  FFT *m_FFT;
  float m_FFTData[FFT::GetFFTSize()]{};
  juce::Colour m_SpectrumColor =
      juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.4f, 0.5f);
  juce::Path m_PathTmp;
  juce::Path m_Path;
};
} // namespace VSTZ::Editor
