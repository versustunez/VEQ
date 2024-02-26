#include "FrequencyResponse.h"

#include "Core/Config.h"
#include "Core/Instance.h"
#include "PluginProcessor.h"
#include "Utils/UI.h"

#include <FMT.h>

namespace VSTZ::Editor {

constexpr size_t SizeResponse = 1000;
FrequencyResponse::FrequencyResponse(InstanceID id) : m_ID(id) {
  auto instance = Core::Instance::get(m_ID);

  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    std::string freq = fmt::format("Band{}_freq", i + 1);
    std::string type = fmt::format("Band{}_type", i + 1);
    std::string qFactor = fmt::format("Band{}_q", i + 1);
    std::string gain = fmt::format("Band{}_gain", i + 1);

    instance->EventHandler.AddHandler(freq, this);
    instance->EventHandler.AddHandler(type, this);
    instance->EventHandler.AddHandler(qFactor, this);
    instance->EventHandler.AddHandler(gain, this);
  }
}

FrequencyResponse::~FrequencyResponse() {
  auto instance = Core::Instance::get(m_ID);

  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    std::string freq = fmt::format("Band{}_freq", i + 1);
    std::string type = fmt::format("Band{}_type", i + 1);
    std::string qFactor = fmt::format("Band{}_q", i + 1);
    std::string gain = fmt::format("Band{}_gain", i + 1);

    instance->EventHandler.RemoveHandler(freq, this);
    instance->EventHandler.RemoveHandler(type, this);
    instance->EventHandler.RemoveHandler(qFactor, this);
    instance->EventHandler.RemoveHandler(gain, this);
  }
}

void FrequencyResponse::paint(juce::Graphics &g) {
  static std::array<juce::Colour, VSTProcessor::Bands> bandColors = {
      juce::Colour(207, 77, 111),  juce::Colour(249, 111, 93),
      juce::Colour(45, 216, 129),  juce::Colour(20, 83, 209),
      juce::Colour(197, 216, 109), juce::Colour(92, 164, 169),
      juce::Colour(191, 6, 3),     juce::Colour(131, 188, 255),
  };
  for (size_t i = 0; i < VSTProcessor::Bands; ++i) {
    g.setColour(bandColors[i]);
    g.strokePath(m_ResponsePaths[i], juce::PathStrokeType(0.3f));
  }

  g.setColour(juce::Colours::orange);
  g.strokePath(m_FullResponse, juce::PathStrokeType(1.0f));
}

void FrequencyResponse::resized() {
  if (m_MagsFullResponse.empty())
    CreateResponseArray();
  PrepareResponse();
}

void FrequencyResponse::Handle(Events::Event *) {
  CreateResponseArray();
  PrepareResponse();
  repaint();
}

void FrequencyResponse::CreateResponseArray() {
  m_Mags.resize(SizeResponse, {0, 0, 0, 0, 0, 0, 0, 0});
  m_MagsFullResponse.resize(SizeResponse);
  double sampleRate = Core::Config::get().sampleRate;
  auto *instance = Core::Instance::get(m_ID);
  auto &bands = instance->Processor->FilterBands;

  const size_t w = SizeResponse;
  const double dw = SizeResponse;

  for (int j = 0; j < w; ++j) {
    m_MagsFullResponse[j] = 1.0;
  }

  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    auto &band = bands[i];
    for (int j = 0; j < w; ++j) {
      const auto freq =
          juce::mapToLog10(static_cast<double>(j) / dw, 20.0, 20000.0);
      auto response =
          band.ApplyingFilter.GetMagnitudeForFrequency(freq, sampleRate);
      m_MagsFullResponse[j] += response;
      m_Mags[j][i] = juce::Decibels::gainToDecibels(response);
    }
  }
}

static float remap(const float i, const float w) {
  return std::lerp(0.0f, w, i / SizeResponse);
}

void FrequencyResponse::PrepareResponse() {
  auto *instance = Core::Instance::get(m_ID);
  auto &bands = instance->Processor->FilterBands;

  auto bounds = getLocalBounds();
  const double outputMin = bounds.getBottom();
  const double outputMax = bounds.getY();

  auto GetScale = [&bands](int index) {
    return Utils::UI::ScaleData{bands[index].Gain, bands[index].Type};
  };
  std::array<Utils::UI::ScaleData, 8> params = {
      GetScale(0), GetScale(1), GetScale(2), GetScale(3),
      GetScale(4), GetScale(5), GetScale(6), GetScale(7),
  };
  const double scale =
      Utils::UI::GetDecibelScaleForArray(params.data(), params.size());
  auto map = [outputMin, outputMax, scale](const double input) -> double {
    return juce::jmap(input, -scale, scale, outputMin, outputMax);
  };

  const auto dw = static_cast<float>(bounds.getWidth());
  int activeBands = 1;

  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    auto &band = bands[i];
    auto &path = m_ResponsePaths[i];
    path.clear();
    if (band.ApplyingFilter.IsBypassed()) {
      continue;
    }
    activeBands++;
    path.startNewSubPath(bounds.getX(), map(m_Mags[0][i] * 0.5));
    for (int j = 1; j < SizeResponse; ++j) {
      path.lineTo(bounds.getX() + remap(j, dw), map(m_Mags[j][i] * 0.5));
    }
  }

  m_FullResponse.clear();
  m_FullResponse.startNewSubPath(
      bounds.getX(),
      map(juce::Decibels::gainToDecibels(m_MagsFullResponse[0] / activeBands)));
  for (int j = 1; j < SizeResponse; ++j) {
    m_FullResponse.lineTo(bounds.getX() + remap(j, dw),
                          map(juce::Decibels::gainToDecibels(
                              m_MagsFullResponse[j] / activeBands)));
  }
}
} // namespace VSTZ::Editor