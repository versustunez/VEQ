#include "FrequencyResponse.h"

#include "Core/Config.h"
#include "Core/Instance.h"
#include "PluginProcessor.h"
#include "Utils/UI.h"

#include <FMT.h>

namespace VSTZ::Editor {
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
  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    g.setColour(bandColors[i]);
    g.strokePath(m_ResponsePaths[i], juce::PathStrokeType(0.3f));
  }
}

void FrequencyResponse::resized() { PrepareResponse(); }

void FrequencyResponse::Handle(Events::Event *) {
  PrepareResponse();
  repaint();
}

void FrequencyResponse::PrepareResponse() {
  std::vector<double> mags;
  auto *instance = Core::Instance::get(m_ID);
  auto &bands = instance->Processor->FilterBands;
  double sampleRate = Core::Config::get().sampleRate;

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
  const double scale = Utils::UI::GetDecibelScaleForArray(params.data(), params.size());
  auto map = [outputMin, outputMax, scale](double input) -> double {
    return juce::jmap(input, -scale, scale, outputMin, outputMax);
  };

  int w = getWidth();
  mags.resize(w);

  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    auto &band = bands[i];
    auto &path = m_ResponsePaths[i];
    for (int j = 0; j < w; ++j) {
      auto freq = juce::mapToLog10(double(j) / (double)w, 20.0, 20000.0);
      if (!band.ApplyingFilter.IsBypassed()) {
        mags[j] = juce::Decibels::gainToDecibels(
            band.ApplyingFilter.GetMagnitudeForFrequency(freq, sampleRate));
      } else {
        break;
      }
    }
    path.clear();
    if (!band.ApplyingFilter.IsBypassed()) {
      path.startNewSubPath(bounds.getX(), map(mags[0]));
      for (int j = 1; j < w; ++j) {
        path.lineTo(bounds.getX() + j, map(mags[j]));
      }
    }
  }
}
} // namespace VSTZ::Editor