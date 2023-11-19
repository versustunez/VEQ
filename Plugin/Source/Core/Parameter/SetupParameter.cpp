#include "Handler.h"
#include "PluginProcessor.h"

#include <FMT.h>

namespace VSTZ::Core {

void ParameterHandler::SetupParameter() {
  constexpr auto frequencyCB = [](float value, float) -> juce::String {
    return juce::String(value, 2) + " Hz";
  };
  constexpr auto decibelCB = [](float value, float) -> juce::String {
    return juce::String(value, 2) + " dB";
  };
  constexpr auto percentCB = [](float value, float) -> juce::String {
    return juce::String(value * 100, 2) + "%";
  };
  constexpr auto typeCB = [](float value, float) -> juce::String {
    switch ((int)value) {
    case 2: return "Peak";
    case 1: return "Low-Shelf";
    case 3: return "High-Shelf";
    }
    return "Off";
  };
  for (int i = 1; i <= VSTProcessor::Bands; ++i) {
    AddParameter({.Name = fmt::format("Band{}_freq", i),
                  .View = fmt::format("Band {} Frequency", i),
                  .Min = 30.0,
                  .Max = 20000.0,
                  .Value = 30.0,
                  .ValueToStringCallback = frequencyCB});
    AddParameter({.Name = fmt::format("Band{}_type", i),
                  .View = fmt::format("Band {} Type", i),
                  .Min = 0.0,
                  .Max = 3.0,
                  .Value = 0.0,
                  .Type = ParameterTypes::Integer,
                  .ValueToStringCallback = typeCB});
    AddParameter({.Name = fmt::format("Band{}_gain", i),
                  .View = fmt::format("Band {} Gain", i),
                  .Min = -30.0,
                  .Max = 30.0,
                  .Value = 0.0,
                  .ValueToStringCallback = decibelCB});
    AddParameter(
        {.Name = fmt::format("Band{}_q", i),
         .View = fmt::format("Band {} Q", i),
         .Min = 0.1,
         .Max = 4.0,
         .Value = 0.7,
         .ValueToStringCallback = [](float value, float) -> juce::String {
           return {value, 2};
         }});
  }
  AddBoolParameter("bypass", "Bypass", false);
  AddBoolParameter("auto_gain", "Auto Gain", false);
  AddBoolParameter("analog", "Analog", false);
  AddParameter({.Name = "analog_strength",
                .View = "Analog Strength",
                .Min = 0.0,
                .Max = 1.0,
                .Value = 0.15,
                .ValueToStringCallback = percentCB});
}
} // namespace VSTZ::Core