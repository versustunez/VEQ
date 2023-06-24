#include "dBMeter.h"

#include "Core/Instance.h"
#include "Utils/UI.h"

#include <FMT.h>

namespace VSTZ::Editor {
DecibelMeter::DecibelMeter(VSTZ::InstanceID id) : m_ID(id) {
  auto instance = Core::Instance::get(m_ID);

  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    std::string freq = fmt::format("Band{}_freq", i + 1);
    std::string type = fmt::format("Band{}_type", i + 1);
    std::string qFactor = fmt::format("Band{}_q", i + 1);
    std::string gain = fmt::format("Band{}_gain", i + 1);

    m_Params[i] = {
        instance->handler->GetParameter(fmt::format("Band{}_type", i + 1)),
        instance->handler->GetParameter(fmt::format("Band{}_freq", i + 1)),
        instance->handler->GetParameter(fmt::format("Band{}_gain", i + 1)),
        instance->handler->GetParameter(fmt::format("Band{}_q", i + 1)),
    };

    instance->EventHandler.AddHandler(freq, this);
    instance->EventHandler.AddHandler(type, this);
    instance->EventHandler.AddHandler(qFactor, this);
    instance->EventHandler.AddHandler(gain, this);
  }
}

DecibelMeter::~DecibelMeter() {
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

void DecibelMeter::paint(juce::Graphics &g) {

  g.setFont(9.0f);
  int increase = m_Scale == 6 ? 3 : 6;
  int range = m_Scale * 2 / increase;
  float current = -m_Scale;
  for (int i = 0; i < range; ++i) {
    if (current == m_Scale || current == -m_Scale) {
      current += increase;
      continue;
    }
    float y = juce::jmap(current, -m_Scale, m_Scale, (float)getHeight(), 0.0f);
    float y2 = juce::jmap(current + increase, -m_Scale, m_Scale,
                          (float)getHeight(), 0.0f);
    g.setColour(juce::Colours::white);
    g.drawText(fmt::format("{:.0f}", current), 0, y - 4.5f, 20, y - y2,
               juce::Justification::centredTop);
    current += increase;
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawLine(20, y, getWidth() - 20, y, 0.2f);
  }
}

void DecibelMeter::Handle(Events::Event *event) {
  auto GetScale = [this](size_t index) {
    return Utils::UI::ScaleData{m_Params[index].Gain, m_Params[index].Type};
  };
  std::array<Utils::UI::ScaleData, 8> params = {
      GetScale(0), GetScale(1), GetScale(2), GetScale(3),
      GetScale(4), GetScale(5), GetScale(6), GetScale(7),
  };
  m_Scale = Utils::UI::GetDecibelScaleForArray(params.data(), params.size());
  repaint();
}
} // namespace VSTZ::Editor