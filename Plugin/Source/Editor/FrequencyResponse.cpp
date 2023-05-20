#include "FrequencyResponse.h"

#include "Core/Config.h"
#include "Core/Instance.h"
#include "PluginProcessor.h"

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
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.87f, 0.01, 1.0f));
  g.strokePath(m_ResponsePath, juce::PathStrokeType(1.0f));
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

  int w = getWidth();
  mags.resize(getWidth());

  for (int i = 0; i < w; ++i) {
    double mag = 1.0;
    auto freq = juce::mapToLog10(double(i) / (double)w, 20.0, 20000.0);
    for (auto band : bands) {
      if (!band.ApplyingFilter.IsBypassed()) {
        mag *= band.ApplyingFilter.GetMagnitudeForFrequency(freq, sampleRate);
      }
    }
    mags[i] = juce::Decibels::gainToDecibels(mag);
  }

  m_ResponsePath.clear();

  auto bounds = getLocalBounds();
  const double outputMin = bounds.getBottom();
  const double outputMax = bounds.getY();
  auto map = [outputMin, outputMax](double input) -> double {
    return juce::jmap(input, -40.0, 40.0, outputMin, outputMax);
  };

  m_ResponsePath.startNewSubPath(bounds.getX(), map(mags[0]));
  for (int i = 1; i < w; ++i) {
    m_ResponsePath.lineTo(bounds.getX() + i, map(mags[i]));
  }
}
} // namespace VSTZ::Editor