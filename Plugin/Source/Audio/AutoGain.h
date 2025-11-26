#pragma once
#include "Structs.h"

#include <array>
#include <vector>

namespace VSTZ {
struct AutoGainer {
  constexpr static size_t BufferSize = 2048;
  AutoGainer();
  AutoGainer(AutoGainer &other) = delete;
  AutoGainer(AutoGainer &&other) = delete;
  AutoGainer &operator=(const AutoGainer &other) = delete;
  AutoGainer &operator=(AutoGainer &&other) = delete;
  ~AutoGainer() = default;

  void providePreSamples(const std::vector<Channel> &buffer, size_t size);
  void providePostSamples(const std::vector<Channel> &buffer, size_t size);

  [[nodiscard]] Channel gainReduction();
  Channel& getBeforeRms();
  void setSampleRate(double sampleRate);


private:
  // Left Right ;)
  Smoother m_Smoother[2]{};
  Channel m_BeforeRMS{0, 0};
  Channel m_GainReduction{1, 1};

  std::array<Channel, BufferSize> m_Pre{};
  std::array<Channel, BufferSize> m_Post{};
  std::size_t m_PreIndex{0}, m_PostIndex{0};
};
} // namespace VSTZ
