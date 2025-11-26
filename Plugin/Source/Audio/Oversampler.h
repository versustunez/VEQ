#pragma once

#include "../Filter.h"
#include "Structs.h"

#include <JuceHeader.h>
#include <vector>

namespace VSTZ {
class Oversampler {
public:
  void SetupMaxSamples(size_t maxSamples);
  void SetupFilter(double sampleRate);

  void SetOversampled(bool);

  void Up(juce::AudioBuffer<float> &);
  void Up(juce::AudioBuffer<double> &);
  void Down(juce::AudioBuffer<float> &);
  void Down(juce::AudioBuffer<double> &);

  void SetChannelData(size_t index, const Channel &channel);
  Channel &GetChannelData(size_t index);
  size_t GetSize() const { return m_ProvidedSamples; }

  std::vector<Channel>& GetBuffer() { return m_Buffer; }
  std::vector<Channel>& GetBufferOriginal() { return m_BufferOriginal; }

  auto begin() { return m_Buffer.begin(); }
  auto end() { return m_Buffer.begin() + m_ProvidedSamples; }

protected:
  void ApplyDown();
  void ApplyUp();
  // we also need our buffer that holds the oversampled data :)
  Filter m_Up{}, m_Down{};
  std::vector<Channel> m_Buffer;
  std::vector<Channel> m_BufferOriginal;
  size_t m_ProvidedSamples{0};
  bool m_Oversampled{false};
};
} // namespace VSTZ