#include <GUI/Theme/Theme.h>
#include <utility>

namespace VSTZ::Theme {
Theme::Theme(const Ref<Core::Properties>& file) : m_configFile(file) {}
Theme::~Theme() {
  for (auto &item : m_colours)
    delete item.second;
}

void Theme::init() {
  getColourFromConfig(Colors::bg);
  getColourFromConfig(Colors::bgTwo);
  getColourFromConfig(Colors::accent);
  getColourFromConfig(Colors::accentTwo);
  getColourFromConfig(Colors::font);

  m_colorMapping["primaryBG"] = Colors::bg;
  m_colorMapping["secondaryBG"] = Colors::bgTwo;
  m_colorMapping["primary"] = Colors::accent;
  m_colorMapping["secondary"] = Colors::accentTwo;
  m_colorMapping["font"] = Colors::font;
}

juce::Colour Theme::getColor(Colors index) {
  if (m_colours[index] != nullptr)
    return *m_colours[index];
  return {255, 255, 255};
}

void Theme::setColor(Colors index, juce::Colour *colour) {
  delete m_colours[index];
  m_colours[index] = colour;
  m_configFile->setValue<juce::String>(colorToString(index),
                                       colour->toDisplayString(true));
}

juce::Colour Theme::getContrastColor(const juce::Colour &colour) {
  size_t r = colour.getRed();
  size_t g = colour.getGreen();
  size_t b = colour.getBlue();
  double val = 0.299 * (r * r) + 0.587 * (g * g) + 0.114 * (b * b);
  constexpr double lightThreshold = 127.5 * 127.5;
  if (val > lightThreshold) {
    return {0, 0, 0};
  }
  return {255, 255, 255};
}

void Theme::getColourFromConfig(Colors index) {
  std::string key = colorToString(index);
  delete m_colours[index];
  auto color = getDefault(index);
  m_colours[index] = new juce::Colour(juce::Colour::fromString(
      m_configFile->asString(key, color.toString().toStdString())));
}

Colors Theme::getColorIndex(const std::string &color) {
  if (m_colorMapping.find(color) != m_colorMapping.end()) {
    return m_colorMapping[color];
  }
  return Colors::unknown;
}

std::string Theme::colorName(Colors index) {
  switch (index) {
  case Colors::bg: return "Primary Background";
  case Colors::bgTwo: return "Secondary Background";
  case Colors::accent: return "Primary Accent";
  case Colors::accentTwo: return "Secondary Accent";
  case Colors::font: return "Font Color";
  case Colors::unknown:
  case Colors::end:
  default: return "Unknown";
  };
}

std::string Theme::colorToString(Colors index) {
  switch (index) {
  case Colors::bg: return "color.primaryBG";
  case Colors::bgTwo: return "color.secondaryBG";
  case Colors::accent: return "color.primary";
  case Colors::accentTwo: return "color.secondary";
  case Colors::font: return "color.font";
  case Colors::unknown:
  case Colors::end:
  default: return "color.unknown";
  }
}

juce::Colour Theme::getDefault(Colors index) {
  switch (index) {
  case Colors::bg: return {27, 27, 33};
  case Colors::bgTwo: return {66, 67, 74};
  case Colors::accent: return {31, 115, 255};
  case Colors::accentTwo: return {44, 40, 93};
  case Colors::font: return {255, 255, 255};
  case Colors::unknown:
  case Colors::end:
  default: return {255, 255, 255};
  }
}
} // namespace VSTZ::Theme
