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
      [this](double) { CalculateWarmthEffect(); });

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
  CalculateWarmthEffect();
}

static float lerp(const float a, const float b, const float alpha) {
  return a + alpha * (b - a);
}

static float driveAmount = std::pow(10.0f, 6.0f / 20.0f);
static float driveAmountReduction = std::pow(10.0f, -6.0f / 20.0f);

static float ApplyAnalogDistortion(const float in,
                                   const float distortionAmount = 0.03) {
  constexpr float clipThreshold = 1.0f;
  const float distortedSignal =
      std::atan(in * driveAmount) * driveAmountReduction;
  const float distortedSample = lerp(in, distortedSignal, distortionAmount);

  return std::clamp(distortedSample, -clipThreshold, clipThreshold);
}

static float ApplySlewLimiter(const float in, float &lastValue,
                              const float slewRate = 0.75) {
  float diff = in - lastValue;
  diff = std::clamp(diff, -slewRate, slewRate);
  lastValue = lastValue + diff;
  return lastValue;
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
  bool useAutoGain = parameters.AutoGain->getBool();
  bool warmth = parameters.Warmth->getBool();

  processor.m_LastValueLeft = dataLeft[0];
  processor.m_LastValueRight = dataRight[0];

  for (int i = 0; i < buffer.getNumSamples(); ++i) {

    int active = 1;
    const auto dLeft = static_cast<float>(dataLeft[i]);
    const auto dRight = static_cast<float>(dataRight[i]);
    processor.instance->InputFFT.PushSample((dLeft + dRight) * 0.5f);
    float lOut = dLeft;
    float rOut = dRight;
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
      lOut = ApplySlewLimiter(
          ApplyAnalogDistortion(lOut, processor.m_AnalogDistortion),
          processor.m_LastValueLeft, processor.m_AnalogSlew);
      rOut = ApplySlewLimiter(
          ApplyAnalogDistortion(rOut, processor.m_AnalogDistortion),
          processor.m_LastValueRight, processor.m_AnalogSlew);
    }
    writeLeft[i] = lOut;
    writeRight[i] = rOut;
  }

  if (useAutoGain) {
    buffer.applyGain(processor.m_AutoGainValue);
  }

  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    processor.instance->OutputFFT.PushSample(
        static_cast<float>(writeLeft[i] + writeRight[i]) * 0.5f);
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
bool VSTProcessor::supportsDoublePrecisionProcessing() const { return true; }
void VSTProcessor::processBlock(juce::AudioBuffer<double> &buffer,
                                juce::MidiBuffer &) {
  ProcessBlock(buffer, *this);
}

void VSTProcessor::CalculateAutoGain() {
  if (!m_Parameters.AutoGain)
    return;

  float dB = 0;
  float active = 1;
  for (auto &FilterBand : FilterBands) {
    dB += FilterBand.ApplyingFilter.IsBypassed() ? 0
                                                 : -FilterBand.Gain->getValue();
    active++;
  }
  dB /= active;
  m_AutoGainValue = std::pow(10.0f, dB / 20.0f);
}

void VSTProcessor::CalculateWarmthEffect() {
  const auto alpha = static_cast<float>(m_Parameters.WarmthEffect->getValue());
  m_AnalogSlew = lerp(0.95, 0.5, alpha);
  m_AnalogDistortion = lerp(0.01, 0.1f, alpha);
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VSTProcessor();
}
