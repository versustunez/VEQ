#include "XYPad.h"

#include "Core/Instance.h"
#include "Utils/UI.h"

#include <FMT.h>

namespace VSTZ::Editor {
XYPad::XYPad(InstanceID id) : m_ID(id) {
  auto *instance = Core::Instance::get(m_ID);
  if (!instance)
    return;
  const std::array bandColors = {
      juce::Colour(207, 77, 111),  juce::Colour(249, 111, 93),
      juce::Colour(45, 216, 129),  juce::Colour(20, 133, 209),
      juce::Colour(197, 216, 109), juce::Colour(92, 164, 169),
      juce::Colour(191, 6, 3),     juce::Colour(131, 188, 255),
  };
  for (size_t i = 0; i < VSTProcessor::Bands; ++i) {
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
  for (size_t i = 0; i < VSTProcessor::Bands; ++i) {
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
          juce::Rectangle(point.X - 5.0f, point.Y - 5.0f, 10.0f, 10.0f));

      if (point.Index == m_Equi->CurrentIndex()) {
        g.drawEllipse(
            juce::Rectangle(point.X - 7.0f, point.Y - 7.0f, 14.0f, 14.0f),
            1.0f);
      }
    }
  }
}

void XYPad::resized() {
  if (m_Scale == 0.0) {
    m_Scale = GetScale();
  }
  for (size_t i = 0; i < VSTProcessor::Bands; ++i) {
    UpdatePoint(i);
  }
}

void XYPad::mouseDown(const juce::MouseEvent &e) {
  float x = e.position.x;
  float y = e.position.y;
  EQPoint *selectedPoint = nullptr;

  for (auto &point : m_Points) {
    if (point.X - 5.0f <= x && x <= point.X + 5.0f && point.Y - 5.0f <= y &&
        y <= point.Y + 5.0f && point.Active) {
      m_CurrentPoint = &point;
      m_Equi->SwitchTo(static_cast<int>(point.Index));
      selectedPoint = &point;
      repaint();
      break;
    }
  }

  if (!selectedPoint && e.getNumberOfClicks() == 1) {
    m_Equi->SwitchTo(-1);
    repaint();
  }

  if (e.getNumberOfClicks() == 2 && m_CurrentPoint) {
    m_CurrentPoint->Parameters.Gain->ResetToDefault();
    m_CurrentPoint->Parameters.Q->ResetToDefault();
  } else if (e.getNumberOfClicks() == 2 && !m_CurrentPoint) {
    for (auto &point : m_Points) {
      if (!point.Active) {
        m_CurrentPoint = &point;
        m_Equi->SwitchTo(static_cast<int>(point.Index));
        if (const float widthPercent = static_cast<float>(getWidth()) / 100.0f;
            x < widthPercent * 4) {
          point.Parameters.Type->SetValueAndNotifyHost(1);
        } else if (x > widthPercent * 96) {
          point.Parameters.Type->SetValueAndNotifyHost(3);
        } else {
          point.Parameters.Type->SetValueAndNotifyHost(2);
        }
        mouseDrag(e);
        break;
      }
    }
  }
}

void XYPad::mouseDrag(const juce::MouseEvent &e) {
  float x = e.position.x;
  float y = e.position.y;
  if (!m_MouseUpdated && m_CurrentPoint) {
    const auto freq = static_cast<float>(
        juce::mapToLog10(x / static_cast<double>(getWidth()), 20.0, 20000.0));
    const float gain =
        juce::jmap(y, 0.0f, static_cast<float>(getHeight()), m_Scale, -m_Scale);

    m_CurrentPoint->Parameters.Frequency->SetValueAndNotifyHost(freq);
    m_CurrentPoint->Parameters.Gain->SetValueAndNotifyHost(gain);
  }
  m_MouseUpdated = false;
}

void XYPad::mouseUp(const juce::MouseEvent &) { m_CurrentPoint = nullptr; }

// @NOTE: This is kinda a mess... we
void XYPad::UpdatePoint(size_t index) {
  const float scale = m_Scale;
  m_Points[index].Active = m_Points[index].Parameters.Type->getInt() != 0;
  const auto freq =
      static_cast<float>(m_Points[index].Parameters.Frequency->getValue());
  const auto gain =
      static_cast<float>(m_Points[index].Parameters.Gain->getValue());

  const int xPos = static_cast<int>(juce::mapFromLog10(freq, 20.0f, 20000.0f) *
                                    static_cast<float>(getWidth()));

  m_Points[index].X = static_cast<float>(xPos);
  m_Points[index].Y =
      juce::jmap(gain, -scale, scale, static_cast<float>(getHeight()), 0.0f);
  repaint();
}

float XYPad::GetScale() const {
  auto GetScale = [this](const size_t index) {
    return Utils::UI::ScaleData{m_Points[index].Parameters.Gain,
                                m_Points[index].Parameters.Type};
  };
  const std::array params = {
      GetScale(0), GetScale(1), GetScale(2), GetScale(3),
      GetScale(4), GetScale(5), GetScale(6), GetScale(7),
  };
  return Utils::UI::GetDecibelScaleForArray(params.data(), params.size());
}

void XYPad::mouseWheelMove(const juce::MouseEvent &,
                           const juce::MouseWheelDetails &wheel) {
  if (m_CurrentPoint) {
    m_CurrentPoint->Parameters.Q->SetValueAndNotifyHost(
        static_cast<float>(m_CurrentPoint->Parameters.Q->getValue()) +
        wheel.deltaY);
  }
}

void XYPad::InternalUpdate(const size_t index) {
  if (const float scale = GetScale(); scale != m_Scale) {
    if (m_CurrentPoint) {
      if (auto *mouseMain = juce::Desktop::getInstance().getDraggingMouseSource(0)) {
        const auto pos = mouseMain->getScreenPosition();
        const auto gain =
            static_cast<float>(m_Points[index].Parameters.Gain->getValue());
        const float newY = juce::jmap(gain, -scale, scale,
                                      static_cast<float>(getHeight()), 0.0f);
        mouseMain->setScreenPosition(
            pos.withY(static_cast<float>(getScreenY()) + newY));
        m_MouseUpdated = true;
      }
    }
    m_Scale = scale;
    resized();
  }
}

void XYPadPointListener::Handle(Events::Event *) {
  m_Pad->InternalUpdate(m_Point->Index);

  m_Pad->UpdatePoint(m_Point->Index);
}
} // namespace VSTZ::Editor
