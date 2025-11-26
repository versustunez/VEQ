#include "PluginProcessor.h"

#include "Core/Config.h"
#include "Core/Instance.h"
#include "PluginEditor.h"
#include "Utils/Audio.h"

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
  m_Parameters.MidSide = instance->handler->GetParameter("mid_side");
  m_Parameters.WarmthEffect =
      instance->handler->GetParameter("analog_strength");
  m_Parameters.WarmthVoltage =
      instance->handler->GetParameter("analog_voltage");
  m_Parameters.Drive = instance->handler->GetParameter("drive");

  m_Parameters.WarmthEffect->RegisterChangeFunction(
      [this](const float value) { m_AnalogMode.CalculateWarmEffect(value); });

  m_Parameters.WarmthVoltage->RegisterChangeFunction(
      [this](const float value) { m_AnalogMode.SetVoltage(value); });

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
  }
  m_AnalogMode.CalculateWarmEffect(m_Parameters.WarmthEffect->getValue());
}

static void ProcessWarmth(VSTProcessor &processor) {
  int active = 1;
  auto &buffer = processor.Oversampler.GetBuffer();
  auto &originalBuffer = processor.Oversampler.GetBufferOriginal();
  for (auto &band : processor.FilterBands) {
    auto &filter = band.ApplyingFilter;
    if (filter.IsBypassed())
      continue;
    active++;
    for (size_t i = 0; i < processor.Oversampler.GetSize(); i++) {
      auto element = originalBuffer[i];
      buffer[i] +=
          {filter.ApplyLeft(element.Left), filter.ApplyRight(element.Right)};
    }
  }

  const double gainReduction = (1.0 / active);
  const bool midSide = processor.GetParameters().MidSide;
  auto drive = VSTZ::AudioUtils::DecibelToGain(processor.GetParameters().Drive->getValue());
  auto& smoother = processor.m_driveSmoother;

  if (midSide) {
    for (auto &element : processor.Oversampler) {
      auto pre = element.midSide();
      auto output =
          processor.m_AnalogMode
              .Apply(pre.Left * gainReduction, pre.Right * gainReduction, smoother.Get(drive))
              .midSide();
      element.Left = output.Left;
      element.Right = output.Right;
    }
  } else {
    for (auto &element : processor.Oversampler) {
      auto output = processor.m_AnalogMode.Apply(
          element.Left * gainReduction, element.Right * gainReduction, smoother.Get(drive));
      element.Left = output.Left;
      element.Right = output.Right;
    }
  }
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
    VSTZ::Channel buf = {buffer.getSample(0, i), buffer.getSample(1, i)};
    processor.instance->InputFFT.PushSample((buf.Left + buf.Right) * 0.5f);
  }

  processor.Oversampler.Up(buffer);
  processor.m_AnalogMode.PreProcess(processor.Oversampler.GetBuffer(),
                                    processor.m_CurrentSamples);
  ProcessWarmth(processor);
  processor.m_AnalogMode.PostProcess(processor.Oversampler.GetBuffer(),
                                     processor.m_CurrentSamples);
  processor.Oversampler.Down(buffer);

  for (size_t i = 0; i < processor.m_CurrentSamples; ++i) {
    VSTZ::Channel buf = {buffer.getSample(0, i), buffer.getSample(1, i)};
    processor.instance->OutputFFT.PushSample((buf.Left + buf.Right) * 0.5f);
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
  if (auto *parameters = xmlState->getChildByName(m_TreeState.state.getType()))
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
    config.sampleRate = sampleRate * 2;
    Oversampler.SetupFilter(sampleRate);
  }
  for (auto &band : FilterBands) {
    band.ApplyingFilter.SetSampleRate((float)sampleRate * 2);
  }
  Oversampler.SetupMaxSamples(samples);
  m_AnalogMode.SetBufferSize(samples * 2); // oversampling
  m_AnalogMode.SetSampleRate(sampleRate * 2);
  m_driveSmoother.SetSampleRate(sampleRate * 2);
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

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VSTProcessor();
}
