#pragma once

#include "BandListener.h"
#include "Events/EventHandler.h"
#include "JuceHeader.h"
#include "ParameterStruct.h"
#include "Audio/AutoGain.h"

#include <Core/Parameter/Parameter.h>

namespace VSTZ::Core {
class Instance;
}
class VSTProcessor : public juce::AudioProcessor {
public:
  VSTProcessor();
  ~VSTProcessor() override;

  void prepareToPlay(double, int) override;
  void releaseResources() override {}

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  void processBlock(juce::AudioBuffer<double> &, juce::MidiBuffer &) override;

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return JucePlugin_Name; }

  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  bool supportsDoublePrecisionProcessing() const override;
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override {
    return juce::String(JucePlugin_Name);
  }
  void changeProgramName(int, const juce::String &) override {}

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  void CalculateAutoGain();
public:
  float m_AutoGainValue{1};
  VSTZ::Parameters& GetParameters() { return m_Parameters; }

public:
  VSTZ::Core::Instance *instance = nullptr;
  constexpr static int Bands{8};
  VSTZ::Band FilterBands[Bands]{};
  VSTZ::Scope<VSTZ::BandListener> BandListener[Bands]{};

private:
  std::string m_Id;
  juce::AudioProcessorValueTreeState m_TreeState;
  VSTZ::Parameters m_Parameters{};
};
