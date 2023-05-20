#include "XYPad.h"

#include "Core/Instance.h"

#include <FMT.h>

namespace VSTZ::Editor {
XYPad::XYPad(VSTZ::InstanceID id) : m_ID(id) {
  auto *instance = Core::Instance::get(m_ID);
  if (!instance)
    return;
  std::array<juce::Colour, VSTProcessor::Bands> bandColors = {
      juce::Colour(207, 77, 111),  juce::Colour(249, 111, 93),
      juce::Colour(45, 216, 129),  juce::Colour(20, 13, 79),
      juce::Colour(197, 216, 109), juce::Colour(92, 164, 169),
      juce::Colour(191, 6, 3),     juce::Colour(131, 188, 255),
  };
  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    m_Points[i].Index = i;
    m_Points[i].Color = bandColors[i];
    m_Points[i].Active = false;
    m_Points[i].Parameters = {
        instance->handler->GetParameter(fmt::format("Band{}_type", i + 1)),
        instance->handler->GetParameter(fmt::format("Band{}_freq", i + 1)),
        instance->handler->GetParameter(fmt::format("Band{}_gain", i + 1)),
        instance->handler->GetParameter(fmt::format("Band{}_q", i + 1)),
    };
    m_PointListener[i].m_Point = &m_Points[i];
    m_PointListener[i].m_Pad = this;
    instance->EventHandler.AddHandler(fmt::format("Band{}_type", i + 1),
                                      &m_PointListener[i]);
    instance->EventHandler.AddHandler(fmt::format("Band{}_freq", i + 1),
                                      &m_PointListener[i]);
    instance->EventHandler.AddHandler(fmt::format("Band{}_gain", i + 1),
                                      &m_PointListener[i]);
  }
}

XYPad::~XYPad() {
  auto *instance = Core::Instance::get(m_ID);
  if (!instance)
    return;
  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    instance->EventHandler.RemoveHandler(fmt::format("Band{}_type", i + 1),
                                         &m_PointListener[i]);
    instance->EventHandler.RemoveHandler(fmt::format("Band{}_freq", i + 1),
                                         &m_PointListener[i]);
    instance->EventHandler.RemoveHandler(fmt::format("Band{}_gain", i + 1),
                                         &m_PointListener[i]);
  }
}

void XYPad::paint(juce::Graphics &g) {
  for (auto &point : m_Points) {
    if (point.Active) {
      g.setColour(point.Color);
      g.fillEllipse(
          juce::Rectangle<float>(point.X - 5.0f, point.Y - 5.0f, 10.0f, 10.0f));
    }
  }
}

void XYPad::resized() {
  for (int i = 0; i < VSTProcessor::Bands; ++i) {
    UpdatePoint(i);
  }
}

void XYPad::mouseDown(const juce::MouseEvent &e) {
  float x = e.position.x;
  float y = e.position.y;
  for (auto &point : m_Points) {
    if (point.X - 5.0f <= x && x <= point.X + 5.0f && point.Y - 5.0f <= y &&
        y <= point.Y + 5.0f && point.Active) {
      m_CurrentPoint = &point;
      m_TabbedComponent->setCurrentTabIndex(point.Index);
      break;
    }
  }
}

void XYPad::mouseDrag(const juce::MouseEvent &e) {
  float x = e.position.x;
  float y = e.position.y;
  if (m_CurrentPoint) {
    auto freq = (float)juce::mapToLog10(x / (double)getWidth(), 20.0, 20000.0);
    float gain = juce::jmap(y, 0.0f, (float)getHeight(), 30.0f, -30.0f);

    m_CurrentPoint->Parameters.Frequency->SetValueAndNotifyHost(freq);
    m_CurrentPoint->Parameters.Gain->SetValueAndNotifyHost(gain);
  }
}

void XYPad::mouseUp(const juce::MouseEvent &e) { m_CurrentPoint = nullptr; }

void XYPad::UpdatePoint(int index) {
  m_Points[index].Active = m_Points[index].Parameters.Type->getInt() != 0;
  auto freq = (float)m_Points[index].Parameters.Frequency->getValue();
  auto gain = (float)m_Points[index].Parameters.Gain->getValue();

  int xPos = juce::mapFromLog10(freq, 20.0f, 20000.0f) * getWidth();

  m_Points[index].X = xPos;
  m_Points[index].Y = juce::jmap(gain, -30.0f, 30.0f, (float)getHeight(), 0.0f);
  repaint();
}

void XYPad::mouseWheelMove(const juce::MouseEvent &event,
                           const juce::MouseWheelDetails &wheel) {
  if (m_CurrentPoint) {
    m_CurrentPoint->Parameters.Q->SetValueAndNotifyHost(
        m_CurrentPoint->Parameters.Q->getValue() + wheel.deltaY);
  }
}

void XYPadPointListener::Handle(Events::Event *event) {
  m_Pad->UpdatePoint(m_Point->Index);
}
} // namespace VSTZ::Editor