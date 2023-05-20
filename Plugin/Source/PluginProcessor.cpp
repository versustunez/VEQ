#include "PluginProcessor.h"

#include "Core/Config.h"
#include "Core/Instance.h"
#include "PluginEditor.h"

#include <FMT.h>

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
}

void VSTProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                juce::MidiBuffer &) {
  juce::ScopedNoDenormals noDenormals;
  for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
    buffer.clear(i, 0, buffer.getNumSamples());
  if (m_Parameters.Bypass->getBool()) {
    return;
  }

  auto mainBuffer = getBusBuffer(buffer, false, 0);
  auto *dataLeft = mainBuffer.getReadPointer(0);
  auto *dataRight = mainBuffer.getReadPointer(1);
  auto *writeLeft = mainBuffer.getWritePointer(0);
  auto *writeRight = mainBuffer.getWritePointer(1);

  double inputGain = 0.0;
  bool useAutoGain = m_Parameters.AutoGain->getBool();

  if (useAutoGain) {
    for (int i = 0; i < buffer.getNumChannels(); ++i)
      inputGain += buffer.getRMSLevel(i, 0, buffer.getNumSamples());
  }

  for (int i = 0; i < mainBuffer.getNumSamples(); ++i) {
    int active = 1;
    float dLeft = dataLeft[i];
    float dRight = dataRight[i];
    for (auto &band : FilterBands) {
      auto &filter = band.ApplyingFilter;
      if (filter.IsBypassed())
        continue;
      writeLeft[i] += filter.ApplyLeft(dLeft);
      writeRight[i] += filter.ApplyRight(dRight);
      active++;
    }
    writeLeft[i] /= active;
    writeRight[i] /= active;
  }

  if (useAutoGain) {
    double outputGain = 0.0;
    for (int i = 0; i < buffer.getNumChannels(); ++i)
      outputGain += buffer.getRMSLevel(i, 0, buffer.getNumSamples());

    auto makeup = (inputGain > 0.0) ? inputGain / outputGain : 1.0;
    for (int i = 0; i < buffer.getNumChannels(); ++i)
      buffer.applyGainRamp(i, 0, buffer.getNumSamples(), m_PreviousMakeup,
                           makeup);
    m_PreviousMakeup = makeup;
  } else {
    m_PreviousMakeup = 0.0f;
  }

  for (int i = 0; i < mainBuffer.getNumSamples(); ++i) {
    instance->LeftFFT.PushSample(writeLeft[i]);
    instance->RightFFT.PushSample(writeRight[i]);
  }

  if (instance->LeftFFT.IsDirty()) {
    auto *editor = dynamic_cast<VSTEditor *>(getActiveEditor());
    if (editor)
      editor->TriggerRepaint();
  }
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
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState == nullptr)
    return;
  auto *parameters = xmlState->getChildByName(m_TreeState.state.getType());
  auto name = xmlState->getStringAttribute("name");
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
  if (config.sampleRate != sampleRate) {
    config.sampleRate = sampleRate;
  }
  for (auto &band : FilterBands) {
    band.ApplyingFilter.SetSampleRate((float)sampleRate);
  }
}
bool VSTProcessor::isBusesLayoutSupported(
    const juce::AudioProcessor::BusesLayout &layouts) const {
  if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled() ||
      layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
    return false;

  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VSTProcessor();
}
