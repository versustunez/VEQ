#include "PluginProcessor.h"

#include "Core/Config.h"
#include "Core/Instance.h"
#include "PluginEditor.h"

#include <FMT.h>
#include <algorithm>

VSTProcessor::VSTProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      instance(VSTZ::Core::Instance::create()),
      m_Id(juce::Uuid().toString().toStdString()),
      m_TreeState(*this, nullptr, JucePlugin_Name,
                  instance->handler->SetupProcessor()) {
  instance->treeState = &m_TreeState;
  instance->Processor = this;
  VSTZ::Core::Config::get().registerProcessor(m_Id, this);
  m_Parameters.Bypass = instance->handler->GetParameter("bypass");
  m_Parameters.AutoGain = instance->handler->GetParameter("auto_gain");
  m_Parameters.Warmth = instance->handler->GetParameter("analog");
  m_Parameters.WarmthEffect =
      instance->handler->GetParameter("analog_strength");

  m_Parameters.WarmthEffect->RegisterChangeFunction(
      [this](const float value) { m_AnalogMode.CalculateWarmEffect(value); });

  m_Parameters.AutoGain->RegisterChangeFunction(
      [&](double) { CalculateAutoGain(); });

  // Set up the Bands Change Listener...
  for (int i = 0; i < Bands; ++i) {
    std::string freq = fmt::format("Band{}_freq", i + 1);
    std::string type = fmt::format("Band{}_type", i + 1);
    std::string qFactor = fmt::format("Band{}_q", i + 1);
    std::string gain = fmt::format("Band{}_gain", i + 1);

    FilterBands[i].Frequency = instance->handler->GetParameter(freq);
    FilterBands[i].Type = instance->handler->GetParameter(type);
    FilterBands[i].Q = instance->handler->GetParameter(qFactor);
    FilterBands[i].Gain = instance->handler->GetParameter(gain);

    BandListener[i].Create(&FilterBands[i]);
    instance->EventHandler.AddHandler(freq, BandListener[i].Get());
    instance->EventHandler.AddHandler(type, BandListener[i].Get());
    instance->EventHandler.AddHandler(qFactor, BandListener[i].Get());
    instance->EventHandler.AddHandler(gain, BandListener[i].Get());

    FilterBands[i].Gain->RegisterChangeFunction(
        [this](double) { CalculateAutoGain(); });
  }
  CalculateAutoGain();
  m_AnalogMode.CalculateWarmEffect(m_Parameters.WarmthEffect->getValue());
}

template <typename T>
static void ProcessBlock(juce::AudioBuffer<T> &buffer,
                         VSTProcessor &processor) {
  juce::ScopedNoDenormals noDenormals;

  for (int i = processor.getTotalNumInputChannels();
       i < processor.getTotalNumOutputChannels(); ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  auto &parameters = processor.GetParameters();
  if (parameters.Bypass->getBool())
    return;

  auto *dataLeft = buffer.getReadPointer(0);
  auto *dataRight = buffer.getReadPointer(1);
  auto *writeLeft = buffer.getWritePointer(0);
  auto *writeRight = buffer.getWritePointer(1);
  bool warmth = parameters.Warmth->getBool();
  // processor.m_AnalogMode.ResetSlew(dataLeft[0], dataRight[0]);
  if (warmth) {
    processor.m_AnalogMode.DriveTarget.CalculateDrive(
        dataLeft, dataRight, static_cast<size_t>(buffer.getNumSamples()));
  }
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    int active = 1;
    auto dLeft = static_cast<float>(dataLeft[i]);
    auto dRight = static_cast<float>(dataRight[i]);
    processor.instance->InputFFT.PushSample((dLeft + dRight) * 0.5f);
    float lOut = dLeft;
    float rOut = dRight;
    if (warmth) {
      const auto processed =
          processor.m_AnalogMode.ApplyPreDistortion(dLeft, dRight);
      dLeft = processed.Left;
      dRight = processed.Right;
    }
    for (auto &band : processor.FilterBands) {
      auto &filter = band.ApplyingFilter;
      if (filter.IsBypassed())
        continue;
      lOut += static_cast<float>(filter.ApplyLeft(dLeft));
      rOut += static_cast<float>(filter.ApplyRight(dRight));
      active++;
    }
    lOut /= static_cast<float>(active);
    rOut /= static_cast<float>(active);

    if (warmth) {
      const auto processed = processor.m_AnalogMode.ApplyPost(lOut, rOut);
      lOut = processed.Left;
      rOut = processed.Right;
    }
    writeLeft[i] = lOut * processor.m_AutoGainValue;
    writeRight[i] = rOut * processor.m_AutoGainValue;
    processor.instance->OutputFFT.PushSample((writeLeft[i] + writeRight[i]) *
                                             0.5f);
  }

  if (processor.instance->InputFFT.IsDirty()) {
    auto *editor = dynamic_cast<VSTEditor *>(processor.getActiveEditor());
    // @Note: This is triggering an async repaint of the editor.
    if (editor)
      editor->TriggerRepaint();
  }
}

void VSTProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                juce::MidiBuffer &) {
  ProcessBlock(buffer, *this);
}

void VSTProcessor::updateTrackProperties(const TrackProperties &properties) {
  instance->state.TrackColor = properties.colour;
}

juce::AudioProcessorEditor *VSTProcessor::createEditor() {
  return new VSTEditor(*this, m_Id);
}

void VSTProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = m_TreeState.copyState();
  auto xmlState = VSTZ::Scope<juce::XmlElement>();
  xmlState.Create("State");
  xmlState->addChildElement(state.createXml().release());
  copyXmlToBinary(*xmlState, destData);
}

void VSTProcessor::setStateInformation(const void *data, int sizeInBytes) {
  std::unique_ptr xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState == nullptr)
    return;
  auto *parameters = xmlState->getChildByName(m_TreeState.state.getType());
  if (parameters)
    m_TreeState.replaceState(juce::ValueTree::fromXml(*parameters));
}

VSTProcessor::~VSTProcessor() {
  VSTZ::Core::Config::get().removeProcessor(m_Id);
  VSTZ::Core::Instance::remove(instance->id);
}

void VSTProcessor::prepareToPlay(double sampleRate, int) {
  auto &config = VSTZ::Core::Config::get();
  // IT'S OKAY BECAUSE THEY ARE LIKELY TO NOT CHANGE OR BE INTEGERS AT ALL.
  if ((int)config.sampleRate != (int)sampleRate) {
    config.sampleRate = sampleRate;
  }
  for (auto &band : FilterBands) {
    band.ApplyingFilter.SetSampleRate((float)sampleRate);
  }
  m_AnalogMode.SetupFilter(sampleRate);
}
bool VSTProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
  if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled() ||
      layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
    return false;

  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}
bool VSTProcessor::supportsDoublePrecisionProcessing() const { return true; }
void VSTProcessor::processBlock(juce::AudioBuffer<double> &buffer,
                                juce::MidiBuffer &) {
  ProcessBlock(buffer, *this);
}

void VSTProcessor::CalculateAutoGain() {
  if (!m_Parameters.AutoGain->getBool()) {
    m_AutoGainValue = 1.0;
    return;
  }
  double dB = 0;
  int active = 1;
  for (auto &FilterBand : FilterBands) {
    if (!FilterBand.ApplyingFilter.IsBypassed()) {
      dB = std::max(dB, FilterBand.Gain->getValue());
      active++;
    }
  }
  dB /= active;
  m_AutoGainValue = std::pow(10.0f, (float)-dB / 20.0f);
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VSTProcessor();
}
