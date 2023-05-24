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
  auto &autoGain = processor.GetAutoGain();
  bool useAutoGain = parameters.AutoGain->getBool();

  if (useAutoGain) {
    autoGain.CalculateInputGain(buffer);
  }

  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    int active = 1;
    float dLeft = dataLeft[i];
    float dRight = dataRight[i];
    for (auto &band : processor.FilterBands) {
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
    autoGain.CalculateOutputGain(buffer);
    for (int i = 0; i < buffer.getNumChannels(); ++i)
      buffer.applyGainRamp(i, 0, buffer.getNumSamples(),
                           autoGain.GetPreviousGain(), autoGain.GetGain());
  }

  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    processor.instance->LeftFFT.PushSample(writeLeft[i]);
    processor.instance->RightFFT.PushSample(writeRight[i]);
  }

  if (processor.instance->LeftFFT.IsDirty()) {
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
  m_AutoGain.SetSampleRate(sampleRate);
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
bool VSTProcessor::supportsDoublePrecisionProcessing() const { return true; }
void VSTProcessor::processBlock(juce::AudioBuffer<double> &buffer,
                                juce::MidiBuffer &) {
  ProcessBlock(buffer, *this);
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VSTProcessor();
}
