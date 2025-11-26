#include "Oversampler.h"

#include <type_traits>

namespace VSTZ {

template <class T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
static size_t CopyBufferPadded(juce::AudioBuffer<T> &buffer,
                               std::vector<Channel> &sampleBuffer) {
  auto *dataLeft = buffer.getReadPointer(0);
  // if we are mono we copy the left channel :)
  auto *dataRight = buffer.getReadPointer(buffer.getNumChannels() == 1 ? 0 : 1);

  size_t realIndex = 0;
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    sampleBuffer[realIndex++] = {
        static_cast<double>(dataLeft[i]),
        static_cast<double>(dataRight[i]),
    };
    sampleBuffer[realIndex++] = {
        0,
        0,
    };
  }
  return realIndex;
}

template <class T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
static size_t CopyBuffer(juce::AudioBuffer<T> &buffer,
                         std::vector<Channel> &sampleBuffer) {
  auto *dataLeft = buffer.getReadPointer(0);
  auto *dataRight = buffer.getReadPointer(buffer.getNumChannels() == 1 ? 0 : 1);

  size_t realIndex = 0;
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    sampleBuffer[realIndex++] = {
        static_cast<double>(dataLeft[i]),
        static_cast<double>(dataRight[i]),
    };
  }
  return buffer.getNumSamples();
}

template <class T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
static void DownSampleCopy(juce::AudioBuffer<T> &buffer,
                           std::vector<Channel> &sampleBuffer,
                           size_t increment = 1) {
  auto *dataLeft = buffer.getWritePointer(0);
  auto *dataRight =
      buffer.getWritePointer(buffer.getNumChannels() == 1 ? 0 : 1);

  size_t realIndex = 0;
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    auto &[Left, Right] = sampleBuffer[realIndex];
    dataLeft[i] = static_cast<T>(Left);
    dataRight[i] = static_cast<T>(Right);
    realIndex += increment;
  }
}

void Oversampler::SetupMaxSamples(const size_t maxSamples) {
  m_Buffer.resize(maxSamples * 2); // 2 Times oversampling ;)
  m_BufferOriginal.resize(maxSamples * 2); // 2 Times oversampling ;)
}
void Oversampler::SetupFilter(double sampleRate) {
  m_Up.SetSampleRate(sampleRate * 2.0);
  m_Down.SetSampleRate(sampleRate * 2.0);

  m_Up.SetFilterType(Filter::Type::LowPass);
  m_Down.SetFilterType(Filter::Type::LowPass);

  m_Up.CalculateCoefficients(0, sampleRate * 0.45, 0.71);
  m_Down.CalculateCoefficients(0, sampleRate * 0.45, 0.71);
}
void Oversampler::SetOversampled(bool value) { m_Oversampled = value; }

void Oversampler::Up(juce::AudioBuffer<float> &buffer) {
  if (m_Oversampled) {
    m_ProvidedSamples = CopyBufferPadded(buffer, m_Buffer);
    ApplyUp();
  } else {
    m_ProvidedSamples = CopyBuffer(buffer, m_Buffer);
  }
  // copy to original buffer before processing
  std::memcpy(m_BufferOriginal.data(), m_Buffer.data(), m_ProvidedSamples * sizeof(Channel));
}
void Oversampler::Up(juce::AudioBuffer<double> &buffer) {
  if (m_Oversampled) {
    m_ProvidedSamples = CopyBufferPadded(buffer, m_Buffer);
    ApplyUp();
  } else {
    m_ProvidedSamples = CopyBuffer(buffer, m_Buffer);
  }
  // copy to original buffer before processing
  std::memcpy(m_BufferOriginal.data(), m_Buffer.data(), m_ProvidedSamples * sizeof(Channel));
}
void Oversampler::Down(juce::AudioBuffer<float> &buffer) {
  ApplyDown();
  DownSampleCopy(buffer, m_Buffer, m_Oversampled ? 2 : 1);
}
void Oversampler::Down(juce::AudioBuffer<double> &buffer) {
  ApplyDown();
  DownSampleCopy(buffer, m_Buffer, m_Oversampled ? 2 : 1);
}
void Oversampler::SetChannelData(const size_t index, const Channel &channel) {
  m_Buffer[index] = channel;
}
Channel &Oversampler::GetChannelData(const size_t index) {
  return m_Buffer[index];
}

void Oversampler::ApplyDown() {
  if (m_Oversampled) {
    for (size_t i = 0; i < m_ProvidedSamples; i++) {
      m_Buffer[i].Left = m_Down.ApplyLeft(m_Buffer[i].Left) * 1.5;
      m_Buffer[i].Right = m_Down.ApplyRight(m_Buffer[i].Right) * 1.5;
    }
  }
}

void Oversampler::ApplyUp() {
  for (size_t i = 0; i < m_ProvidedSamples; i++) {
    m_Buffer[i].Left = m_Up.ApplyLeft(m_Buffer[i].Left);
    m_Buffer[i].Right = m_Up.ApplyRight(m_Buffer[i].Right);
  }
}

} // namespace VSTZ