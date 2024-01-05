#include "BandListener.h"

namespace VSTZ {

void BandListener::Handle(Events::Event *event) {
  auto *changeEvent = event->As<Events::ParameterChange>();
  if (changeEvent == nullptr)
    return;
  auto *param = changeEvent->Parameter;
  if (param == m_Band->Type) {
    m_Band->ApplyingFilter.Reset();
    int type = param->getInt();
    switch (type) {
    case 2: m_Band->ApplyingFilter.SetFilterType(Filter::Type::Peak); break;
    case 1: m_Band->ApplyingFilter.SetFilterType(Filter::Type::LowShelf); break;
    case 3:
      m_Band->ApplyingFilter.SetFilterType(Filter::Type::HighShelf);
      break;
    case 4: m_Band->ApplyingFilter.SetFilterType(Filter::Type::LowPass); break;
    case 5: m_Band->ApplyingFilter.SetFilterType(Filter::Type::HighPass); break;

    default: m_Band->ApplyingFilter.SetFilterType(Filter::Type::NO); break;
    }
  }
  m_Band->ApplyingFilter.CalculateCoefficients(m_Band->Gain->getValue(),
                                               m_Band->Frequency->getValue(),
                                               m_Band->Q->getValue());
}
} // namespace VSTZ