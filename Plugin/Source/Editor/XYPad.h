#pragma once

#include "Editor/EQUI.h"
#include "GUI/Components/VComponent.h"
#include "PluginProcessor.h"
#include "TypeDefs.h"

#include <array>

namespace VSTZ::Editor {

struct EQPointParameters {
  Core::Parameter *Type{nullptr};
  Core::Parameter *Frequency{nullptr};
  Core::Parameter *Gain{nullptr};
  Core::Parameter *Q{nullptr};
};

struct EQPoint {
  size_t Index{0};
  float X{0}, Y{0};
  juce::Colour Color{juce::Colours::green};
  bool Active{false};
  EQPointParameters Parameters{};
};

class XYPad;

struct XYPadPointListener : public Events::Handler {
  XYPad *m_Pad{nullptr};
  void Handle(Events::Event *event) override;
  EQPoint *m_Point{nullptr};
};

class XYPad : public GUI::VComponent {
public:
  XYPad(InstanceID id);
  ~XYPad();

  void SetEQUI(EQUI *equi) {
    m_Equi = equi;
  }

  void paint(juce::Graphics &g) override;
  void resized() override;

  void UpdatePoint(size_t index);

  void mouseDown(const juce::MouseEvent &e) override;
  void mouseDrag(const juce::MouseEvent &e) override;
  void mouseUp(const juce::MouseEvent &e) override;
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;

  void InternalUpdate(size_t index);

protected:
  float GetScale() const;

protected:
  float m_Scale{0.0f};
  EQPoint *m_CurrentPoint{nullptr};
  bool m_MouseUpdated{false};

  InstanceID m_ID{0};
  EQUI *m_Equi{nullptr};
  std::array<EQPoint, VSTProcessor::Bands> m_Points{};
  std::array<XYPadPointListener, VSTProcessor::Bands> m_PointListener{};
};
} // namespace VSTZ::Editor
