#pragma once

#include "Events/EventHandler.h"
#include "ParameterStruct.h"

namespace VSTZ {

class BandListener : public Events::Handler {
public:
  BandListener(Band *band)
      : m_Band(band){}

  void Handle(Events::Event *event) override;

protected:
  Band *m_Band;
};

} // namespace VSTZ
