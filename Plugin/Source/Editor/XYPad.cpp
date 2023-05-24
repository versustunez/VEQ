#include "XYPad.h"

#include "Core/Instance.h"
#include "Utils/UI.h"

#include <FMT.h>

namespace VSTZ::Editor {
XYPad::XYPad(VSTZ::InstanceID id) : m_ID(id) {
  auto *instance = Core::Instance::get(m_ID);
  if (!instance)
    return;
  std::array<juce::Colour, VSTProcessor::Bands> bandColors = {
      juce::Colour(207, 77, 111),  juce::Colour(249, 111, 93),
      juce::Colour(45, 216, 129),  juce::Colour(20, 83, 209),
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
          juce::Rectangle<float>(point.X - 5.0f, point.Y - 5.0f, 10.0f, 10.0f));
    }
  }
}

void XYPad::resized() {
  for (size_t i = 0; i < VSTProcessor::Bands; ++i) {
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
      m_TabbedComponent->setCurrentTabIndex((int)point.Index);
      break;
    }
  }
  if (e.getNumberOfClicks() == 2 && m_CurrentPoint) {
    m_CurrentPoint->Parameters.Gain->ResetToDefault();
    m_CurrentPoint->Parameters.Q->ResetToDefault();
  }

  if (e.getNumberOfClicks() == 2 && !m_CurrentPoint) {
    for (auto &point : m_Points) {
      if (!point.Active) {
        m_CurrentPoint = &point;
        m_TabbedComponent->setCurrentTabIndex((int)point.Index);
        float widthPercent = (float)getWidth() / 100.0f;
        if (x < widthPercent * 4) {
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
    auto freq = (float)juce::mapToLog10(x / (double)getWidth(), 20.0, 20000.0);
    float gain = juce::jmap(y, 0.0f, (float)getHeight(), m_Scale, -m_Scale);

    m_CurrentPoint->Parameters.Frequency->SetValueAndNotifyHost(freq);
    m_CurrentPoint->Parameters.Gain->SetValueAndNotifyHost(gain);
  }
  m_MouseUpdated = false;
}

void XYPad::mouseUp(const juce::MouseEvent &) { m_CurrentPoint = nullptr; }

void XYPad::UpdatePoint(size_t index) {
  float scale = GetScale();
  if (scale != m_Scale) {
    if (m_CurrentPoint) {
      auto *mouseMain = juce::Desktop::getInstance().getMouseSource(0);
      if (mouseMain) {
        auto pos = mouseMain->getScreenPosition();
        mouseMain->setScreenPosition(
            pos.withY((float)getScreenY() + m_Points[index].Y));
        m_MouseUpdated = true;
      }
    }
    m_Scale = scale;
    resized();
    return;
  }

  m_Points[index].Active = m_Points[index].Parameters.Type->getInt() != 0;
  auto freq = (float)m_Points[index].Parameters.Frequency->getValue();
  auto gain = (float)m_Points[index].Parameters.Gain->getValue();

  int xPos = int(juce::mapFromLog10(freq, 20.0f, 20000.0f) * (float)getWidth());

  m_Points[index].X = (float)xPos;
  m_Points[index].Y = juce::jmap(gain, -scale, scale, (float)getHeight(), 0.0f);
  repaint();
}
float XYPad::GetScale() {
  auto GetScale = [this](size_t index) {
    return Utils::UI::ScaleData{m_Points[index].Parameters.Gain,
                                m_Points[index].Parameters.Type};
  };
  std::array<Utils::UI::ScaleData, 8> params = {
      GetScale(0), GetScale(1), GetScale(2), GetScale(3),
      GetScale(4), GetScale(5), GetScale(6), GetScale(7),
  };
  return Utils::UI::GetDecibelScaleForArray(params.data(), params.size());
}

void XYPad::mouseWheelMove(const juce::MouseEvent &,
                           const juce::MouseWheelDetails &wheel) {
  if (m_CurrentPoint) {
    m_CurrentPoint->Parameters.Q->SetValueAndNotifyHost(
        (float)m_CurrentPoint->Parameters.Q->getValue() + wheel.deltaY);
  }
}

void XYPadPointListener::Handle(Events::Event *) {
  m_Pad->UpdatePoint(m_Point->Index);
}
} // namespace VSTZ::Editor