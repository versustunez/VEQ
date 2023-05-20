#pragma once

#include "Audio/FFT.h"
#include "GUI/Components/VComponent.h"

#include <TypeDefs.h>

namespace VSTZ::Editor {
class Spectrum : public GUI::VComponent {
public:
  explicit Spectrum(InstanceID id, FFT *usingFFT) : m_ID(id), m_FFT(usingFFT){};
  void paint(juce::Graphics &g) override;

  void SetColor(float red, float green, float blue) {
    m_SpectrumColor = juce::Colour::fromFloatRGBA(red, green, blue, 0.5f);
  }

protected:
  void PrepareFFT();

protected:
  InstanceID m_ID;
  FFT *m_FFT;
  float m_FFTData[FFT::GetFFTSize()]{};
  juce::Colour m_SpectrumColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.4f, 0.5f);
};
} // namespace VSTZ::Editor
