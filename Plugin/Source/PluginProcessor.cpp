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

  auto &parameters = processor.GetParameters();
  if (parameters.Bypass->getBool())
    return;

  processor.m_CurrentSamples = buffer.getNumSamples();
  for (size_t i = 0; i < processor.m_CurrentSamples; ++i) {
    auto &buf = processor.Buffer[i];
    buf.Left = buffer.getSample(0, i);
    buf.Right = buffer.getSample(1, i);
    processor.instance->InputFFT.PushSample((buf.Left + buf.Right) * 0.5f);
  }

  bool warmth = parameters.Warmth->getBool();
  if (warmth) {
    processor.m_AnalogMode.PreProcess(processor.Buffer,
                                      processor.m_CurrentSamples);
    ProcessWarmth(processor, buffer.getWritePointer(0),
                  buffer.getWritePointer(1));
  } else {
    ProcessNormal(processor, buffer.getWritePointer(0),
                  buffer.getWritePointer(1));
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

void VSTProcessor::prepareToPlay(double sampleRate, int samples) {
  auto &config = VSTZ::Core::Config::get();
  // IT'S OKAY BECAUSE THEY ARE LIKELY TO NOT CHANGE OR BE INTEGERS AT ALL.
  if ((int)config.sampleRate != (int)sampleRate) {
    config.sampleRate = sampleRate;
  }
  for (auto &band : FilterBands) {
    band.ApplyingFilter.SetSampleRate((float)sampleRate);
  }
  m_AnalogMode.Resize(samples);
  m_AnalogMode.SetupFilter(sampleRate);

  Buffer.resize(samples);
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

template <typename T>
static void ProcessWarmth(VSTProcessor &processor, T *leftBuffer,
                          T *rightBuffer) {
  auto &analog = processor.m_AnalogMode;
  int active = 1;
  for (auto &band : processor.FilterBands) {
    auto &filter = band.ApplyingFilter;
    if (filter.IsBypassed())
      continue;
    active++;
    for (int i = 0; i < processor.m_CurrentSamples; ++i) {
      leftBuffer[i] +=
          static_cast<float>(filter.ApplyLeft(analog.m_BufferLeft[i]));
      rightBuffer[i] +=
          static_cast<float>(filter.ApplyRight(analog.m_BufferRight[i]));
    }
  }

  const double gainReduction = (1.0 / active) * processor.m_AutoGainValue;
  for (int i = 0; i < processor.m_CurrentSamples; ++i) {
    auto val = analog.ApplyPost(leftBuffer[i] * gainReduction,
                                rightBuffer[i] * gainReduction);
    leftBuffer[i] = val.Left;
    rightBuffer[i] = val.Right;
    processor.instance->OutputFFT.PushSample((leftBuffer[i] + rightBuffer[i]) *
                                             0.5f);
  }
}
template <typename T>
static void ProcessNormal(VSTProcessor &processor, T *leftBuffer,
                          T *rightBuffer) {
  int active = 1;
  for (auto &band : processor.FilterBands) {
    auto &filter = band.ApplyingFilter;
    if (filter.IsBypassed())
      continue;
    active++;
    for (int i = 0; i < processor.m_CurrentSamples; ++i) {
      auto &el = processor.Buffer[i];
      leftBuffer[i] += filter.ApplyLeft(el.Left);
      rightBuffer[i] += filter.ApplyRight(el.Right);
    }
  }

  const double gainReduction = (1.0 / active) * processor.m_AutoGainValue;
  for (int i = 0; i < processor.m_CurrentSamples; ++i) {
    leftBuffer[i] *= gainReduction;
    rightBuffer[i] *= gainReduction;
    processor.instance->OutputFFT.PushSample((leftBuffer[i] + rightBuffer[i]) *
                                             0.5f);
  }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VSTProcessor();
}
