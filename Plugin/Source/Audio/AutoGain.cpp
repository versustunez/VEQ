#include "AutoGain.h"

#include <cmath>

namespace VSTZ {

static Channel
calculateRMS(std::array<Channel, AutoGainer::BufferSize> &array) {
  Channel rms = {0, 0};
  for (auto& current : array) {
    rms.Left += current.Left * current.Left;
    rms.Right += current.Right * current.Right;
  }
  constexpr static double size = AutoGainer::BufferSize;
  rms = {
      std::sqrt(rms.Left / size),
      std::sqrt(rms.Right / size),
  };
  return rms.toDecibels();
}

AutoGainer::AutoGainer() {
  m_Smoother[0].Previous = 1.0;
  m_Smoother[1].Previous = 1.0;
}
void AutoGainer::providePreSamples(const std::vector<Channel> &buffer,
                                   size_t size) {
  for (size_t i = 0; i < size; i++) {
    m_Pre[m_PreIndex++] = buffer[i];
    if (m_PreIndex == BufferSize) {
      m_PreIndex = 0;
      m_BeforeRMS = calculateRMS(m_Pre);
    }
  }
}

void AutoGainer::providePostSamples(const std::vector<Channel> &buffer,
                                    size_t size) {
  for (size_t i = 0; i < size; i++) {
    m_Post[m_PostIndex++] = buffer[i];
    if (m_PostIndex == BufferSize) {
      m_PostIndex = 0;
      Channel gainReduction = m_BeforeRMS - calculateRMS(m_Post);
      m_GainReduction = gainReduction.toGain();
    }
  }
}
Channel AutoGainer::gainReduction() {
  return {
      m_Smoother[0].Get(m_GainReduction.Left),
      m_Smoother[1].Get(m_GainReduction.Right),
  };
}

Channel& AutoGainer::getBeforeRms() {
  return m_BeforeRMS;
}

void AutoGainer::setSampleRate(double sampleRate) {
  m_Smoother[0].SetSampleRate(sampleRate);
  m_Smoother[1].SetSampleRate(sampleRate);
}
} // namespace VSTZ